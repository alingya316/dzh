/// @file WordProvider.cpp
/// LLM 请求与本地词库回退；失败原因可通过 stderr 的 WordProvider[LLM] 诊断行查看。

#include "System/WordProvider.h"

#include <QCoreApplication>
#include <QFile>
#include <QIODevice>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRandomGenerator>
#include <QUrl>

#include <cstdio>

namespace {

/// 打到 stderr，便于 word_llm_smoke 或终端排查网络/解析问题。
void llmDiag(const char* tag, const QString& msg) {
    const QString line = QStringLiteral("WordProvider[LLM] ") + QString::fromUtf8(tag) + QStringLiteral(": ") + msg + QLatin1Char('\n');
    const QByteArray utf8 = line.toUtf8();
    std::fwrite(utf8.constData(), 1, static_cast<size_t>(utf8.size()), stderr);
    std::fflush(stderr);
}

struct LlmConnection {
    QString apiUrl;
    QString apiKey;
    QString model;
};

const QString kDashScopeBeijingCompatible =
    QStringLiteral("https://dashscope.aliyuncs.com/compatible-mode/v1/chat/completions");

QByteArray stripUtf8Bom(QByteArray raw) {
    if (raw.size() >= 3 && static_cast<unsigned char>(raw[0]) == 0xEF && static_cast<unsigned char>(raw[1]) == 0xBB
        && static_cast<unsigned char>(raw[2]) == 0xBF) {
        raw.remove(0, 3);
    }
    return raw;
}

/// 先读 applicationDirPath()/llm_config.json，缺项再用环境变量补全（含阿里云 DASHSCOPE_API_KEY）
LlmConnection loadLlmConnection() {
    LlmConnection c;
    const QString path = QCoreApplication::applicationDirPath() + QStringLiteral("/llm_config.json");
    QFile file(path);
    if (file.open(QIODevice::ReadOnly)) {
        QJsonParseError pe{};
        const QJsonDocument doc = QJsonDocument::fromJson(stripUtf8Bom(file.readAll()), &pe);
        if (doc.isObject()) {
            const QJsonObject o = doc.object();
            c.apiUrl = o.value(QStringLiteral("apiUrl")).toString().trimmed();
            c.apiKey = o.value(QStringLiteral("apiKey")).toString().trimmed();
            c.model = o.value(QStringLiteral("model")).toString().trimmed();
        } else if (pe.error != QJsonParseError::NoError) {
            llmDiag("config", path + QStringLiteral(" JSON parse error: ") + pe.errorString());
        }
    }
    const QByteArray spaceKeyEnv = qgetenv("SPACE_LLM_API_KEY");
    const QByteArray dashKeyEnv = qgetenv("DASHSCOPE_API_KEY");

    if (c.apiUrl.isEmpty()) {
        c.apiUrl = QString::fromLocal8Bit(qgetenv("SPACE_LLM_API_URL")).trimmed();
    }
    if (c.apiUrl.isEmpty()) {
        c.apiUrl = QString::fromUtf8(qgetenv("DASHSCOPE_COMPATIBLE_URL")).trimmed();
    }

    if (c.apiKey.isEmpty()) {
        c.apiKey = QString::fromUtf8(spaceKeyEnv).trimmed();
    }
    if (c.apiKey.isEmpty()) {
        c.apiKey = QString::fromUtf8(dashKeyEnv).trimmed();
    }

    // 与 Go 示例一致：仅配置了百炼环境变量 Key、未指定 URL 时使用北京兼容模式端点
    if (c.apiUrl.isEmpty() && !c.apiKey.isEmpty() && !dashKeyEnv.isEmpty()
        && c.apiKey == QString::fromUtf8(dashKeyEnv).trimmed() && spaceKeyEnv.isEmpty()) {
        c.apiUrl = kDashScopeBeijingCompatible;
    }

    if (c.model.isEmpty()) {
        c.model = QString::fromLocal8Bit(qgetenv("SPACE_LLM_MODEL")).trimmed();
    }
    if (c.model.isEmpty()) {
        c.model = QString::fromUtf8(qgetenv("DASHSCOPE_MODEL")).trimmed();
    }
    return c;
}


/// 从 assistant 文本中解析 {"word":"..."} 或首行纯单词
QString parseWordFromAssistantContent(const QString& raw) {
    QString t = raw.trimmed();
    if (t.startsWith(QStringLiteral("```"))) {
        const int brace0 = t.indexOf('{');
        const int brace1 = t.lastIndexOf('}');
        if (brace0 >= 0 && brace1 > brace0) {
            t = t.mid(brace0, brace1 - brace0 + 1);
        }
    }
    QJsonParseError err{};
    const QJsonDocument jd = QJsonDocument::fromJson(t.toUtf8(), &err);
    if (jd.isObject()) {
        const QString w = WordProvider::normalizeWord(jd.object().value(QStringLiteral("word")).toString());
        if (!w.isEmpty()) {
            return w;
        }
    }
    const QString firstLine = t.split(QChar('\n')).value(0).trimmed();
    return WordProvider::normalizeWord(firstLine);
}

/// 读取 OpenAI 兼容响应 choices[0].message.content，交给 parseWordFromAssistantContent。
QString parseWordFromChatCompletionJson(const QByteArray& data) {
    const QJsonDocument root = QJsonDocument::fromJson(data);
    if (!root.isObject()) {
        return {};
    }
    const QJsonObject o = root.object();
    if (o.contains(QStringLiteral("error"))) {
        return {};
    }
    const QJsonArray choices = o.value(QStringLiteral("choices")).toArray();
    if (choices.isEmpty()) {
        return {};
    }
    const QJsonObject msg = choices.at(0).toObject().value(QStringLiteral("message")).toObject();
    const QString content = msg.value(QStringLiteral("content")).toString();
    return parseWordFromAssistantContent(content);
}

} // namespace

QString WordProvider::normalizeWord(const QString& raw) {
    QString w = raw.trimmed().toLower();
    QString only;
    only.reserve(w.size());
    for (QChar c : w) {
        if (c >= 'a' && c <= 'z') only.append(c);
    }
    if (only.size() < 4 || only.size() > 24) return {};
    return only;
}

WordProvider::WordProvider(QObject* parent)
    : QObject(parent)
    , m_net(new QNetworkAccessManager(this)) {
    m_localWords = QStringList{
        "about", "above", "accept", "across", "action", "active", "advance",
        "advice", "affair", "afraid", "again", "against", "almost", "alone",
        "along", "always", "amount", "ancient", "angel", "anger", "animal",
        "annual", "answer", "anyone", "anything", "appeal", "appear", "apple",
        "arrive", "artist", "assume", "attack", "attempt", "attend", "author",
        "avenue", "battle", "beauty", "become", "before", "behalf", "behave",
        "behind", "belief", "believe", "belong", "benefit", "beside", "beyond",
        "bitter", "blame", "blood", "bother", "bottle", "bottom", "brain",
        "branch", "brave", "bridge", "bright", "broken", "brother", "budget",
        "bullet", "camera", "cancel", "candle", "captain", "carbon", "career",
        "castle", "cattle", "center", "chance", "change", "channel", "charge",
        "charm", "choice", "choose", "church", "circle", "citizen", "climate",
        "clinic", "cloud", "coffee", "collar", "college", "combat", "comfort",
        "commit", "company", "compare", "compete", "complex", "concern", "confirm",
        "connect", "contact", "contain", "content", "control", "convert", "cookie",
        "corner", "correct", "cottage", "cotton", "council", "country", "couple",
        "courage", "course", "cousin", "create", "credit", "crisis", "culture",
        "curious", "current", "custom", "damage", "danger", "debate", "decade",
        "decide", "declare", "decline", "defeat", "defend", "define", "degree",
        "delight", "deliver", "demand", "depend", "desert", "deserve", "design",
        "desire", "destroy", "detail", "develop", "device", "dialog", "differ",
        "dinner", "direct", "discover", "discuss", "disease", "display", "distance",
        "divide", "doctor", "dollar", "double", "dragon", "driver", "during",
        "eager", "eastern", "economy", "effect", "effort", "either", "elect",
        "element", "emerge", "emotion", "emperor", "employ", "enable", "enemy",
        "energy", "engage", "engine", "enhance", "enjoy", "enough", "ensure",
        "entire", "escape", "evening", "examine", "example", "excite", "excuse",
        "exhibit", "exist", "expand", "expect", "expense", "expert", "explain",
        "explode", "explore", "export", "expose", "express", "extend", "extreme",
        "fabric", "factor", "failure", "family", "fantasy", "fashion", "father",
        "fault", "favor", "fellow", "female", "figure", "finger", "finish",
        "flight", "flower", "follow", "forest", "forget", "forgive", "formal",
        "former", "fortune", "forward", "freedom", "freeze", "friend", "fright",
        "galaxy", "garden", "gather", "general", "gentle", "genuine", "global",
        "golden", "govern", "growth", "handle", "happen", "harbor", "health",
        "heaven", "hidden", "honest", "honor", "horror", "hunger", "ignore",
        "imagine", "impact", "import", "improve", "include", "income", "increase",
        "indeed", "inform", "injure", "insect", "inside", "insist", "inspire",
        "instant", "instead", "intend", "interest", "invade", "invent", "invest",
        "invite", "involve", "isolate", "jacket", "journey", "judgment", "jungle",
        "junior", "justice", "kitchen", "ladder", "landscape", "language",
        "lantern", "launch", "lawyer", "leader", "league", "legend", "length",
        "lesson", "letter", "liberty", "library", "listen", "lonely", "loyal",
        "luggage", "luxury", "machine", "magazine", "magnet", "manage", "manner",
        "marble", "margin", "market", "marriage", "master", "matter", "maximum",
        "measure", "medical", "medium", "member", "memory", "mental", "mention",
        "merchant", "mercy", "merely", "message", "meteor", "method", "mighty",
        "military", "mineral", "minor", "minute", "miracle", "mirror", "missile",
        "mission", "mistake", "mixture", "mobile", "model", "modern", "modest",
        "moment", "monitor", "monster", "motion", "motive", "mountain", "mourn",
        "murder", "muscle", "museum", "mystery", "narrow", "nation", "native",
        "natural", "nearby", "nebula", "nervous", "network", "neutral", "nothing",
        "notice", "notion", "novel", "nuclear", "object", "observe", "obtain",
        "obvious", "offend", "office", "operate", "opinion", "oppose", "option",
        "orange", "orbit", "outcome", "outdoor", "outline", "output", "overcome",
        "oxygen", "palace", "parade", "parent", "partner", "passage", "passion",
        "patent", "patience", "patient", "pattern", "payment", "percent", "perfect",
        "perform", "perhaps", "period", "permit", "persist", "picture", "pillow",
        "pioneer", "planet", "plastic", "player", "pleasure", "pledge", "pocket",
        "poetry", "police", "policy", "polish", "polite", "popular", "portion",
        "portrait", "possess", "potato", "poverty", "powder", "praise", "prayer",
        "precious", "predict", "prefer", "prepare", "present", "preserve", "pretend",
        "prevent", "primary", "prince", "prison", "private", "problem", "proceed",
        "produce", "product", "professor", "profit", "progress", "project", "promise",
        "promote", "proper", "protect", "prove", "provide", "publish", "punish",
        "purpose", "pursue", "puzzle", "qualify", "quality", "quarter", "question",
        "rabbit", "racial", "random", "rapid", "rather", "realize", "reason",
        "receive", "recent", "recipe", "record", "recover", "reduce", "reflect",
        "refuse", "regard", "region", "regret", "reject", "relate", "release",
        "relief", "religion", "remain", "remark", "remember", "remind", "remove",
        "replace", "report", "request", "require", "rescue", "research", "resist",
        "resolve", "respond", "restore", "result", "retain", "retire", "reveal",
        "revenge", "reverse", "review", "reward", "rocket", "romantic", "routine",
        "royal", "sacred", "salary", "satellite", "satisfy", "scatter", "scenery",
        "schedule", "scholar", "science", "screen", "search", "season", "secret",
        "secure", "select", "selfish", "senior", "sentence", "separate", "serious",
        "servant", "service", "settle", "severe", "shadow", "shelter", "shield",
        "signal", "silence", "silver", "similar", "simple", "sincere", "skeleton",
        "slogan", "smooth", "social", "society", "soldier", "solid", "solution",
        "sorrow", "source", "special", "species", "speech", "spirit", "splendid",
        "squeeze", "stable", "stadium", "standard", "station", "statue", "steady",
        "stomach", "storage", "strange", "stream", "strength", "stretch", "strike",
        "struggle", "student", "subject", "submit", "succeed", "suffer", "suggest",
        "suitable", "summit", "supply", "support", "suppose", "surface", "surgery",
        "surprise", "surrender", "surround", "survive", "suspect", "sustain",
        "symbol", "sympathy", "symptom", "system", "tackle", "talent", "target",
        "temple", "tendency", "tension", "terminal", "territory", "terror", "theater",
        "theory", "therapy", "thought", "threat", "throne", "thunder", "ticket",
        "tobacco", "tolerate", "tongue", "tourist", "toward", "tradition", "tragedy",
        "training", "transfer", "transport", "treasure", "treaty", "tremble",
        "trial", "tribute", "trigger", "triumph", "trouble", "trust", "truth",
        "tunnel", "tutor", "typical", "ultimate", "undergo", "unfair", "uniform",
        "unique", "universe", "unknown", "unless", "unusual", "update", "upgrade",
        "urgent", "useful", "vacation", "valuable", "variety", "vehicle", "venture",
        "version", "veteran", "victim", "victory", "village", "violence", "virtue",
        "visible", "vision", "visitor", "visual", "vivid", "volcano", "volume",
        "voyage", "wander", "warmth", "wealth", "weapon", "weather", "wedding",
        "weight", "welfare", "western", "whether", "whisper", "willing", "winner",
        "wisdom", "witness", "wonder", "worship", "worthy", "wound", "writing",
        "youth", "zealous",
    };
}

QString WordProvider::pickLocalWord() {
    if (m_localWords.isEmpty()) {
        return "space";
    }
    const QString w = m_localWords.at(m_localIndex % m_localWords.size());
    ++m_localIndex;
    return w;
}

QString WordProvider::pickLocalWordAvoidingUsed() {
    if (m_localWords.isEmpty()) {
        return QStringLiteral("space");
    }
    const int n = m_localWords.size();
    for (int i = 0; i < n; ++i) {
        const QString w = m_localWords.at(m_localIndex % n);
        ++m_localIndex;
        const QString k = WordProvider::normalizeWord(w);
        const QString key = k.isEmpty() ? w.trimmed().toLower() : k;
        if (!key.isEmpty() && !m_usedApiWords.contains(key)) {
            return w;
        }
    }
    return pickLocalWord();
}

void WordProvider::rememberShownWord(const QString& word) {
    const QString k = WordProvider::normalizeWord(word);
    const QString key = k.isEmpty() ? word.trimmed().toLower() : k;
    if (key.isEmpty()) {
        return;
    }
    if (m_usedApiWords.contains(key)) {
        return;
    }
    m_usedApiWords.insert(key);
    m_usedApiChrono.append(key);
}

void WordProvider::requestWord() {
    const LlmConnection conn = loadLlmConnection();
    if (conn.apiUrl.isEmpty() || conn.apiKey.isEmpty()) {
        const QString cfgPath = QCoreApplication::applicationDirPath() + QStringLiteral("/llm_config.json");
        llmDiag("config",
            QStringLiteral("apiUrl or apiKey empty. Expected llm_config.json next to exe: ") + cfgPath
                + QStringLiteral(" (exists: ") + (QFile::exists(cfgPath) ? QStringLiteral("yes") : QStringLiteral("no"))
                + QStringLiteral("). Or set env DASHSCOPE_API_KEY / SPACE_LLM_API_KEY."));
        QString w = pickLocalWordAvoidingUsed();
        rememberShownWord(w);
        emit wordReady(w, false);
        return;
    }
    QString model = conn.model;
    if (model.isEmpty()) {
        if (conn.apiUrl.contains(QStringLiteral("dashscope"), Qt::CaseInsensitive)) {
            model = QStringLiteral("qwen-plus");
        } else {
            model = QStringLiteral("gpt-4o-mini");
        }
    }
    m_wordRequestAttempt = 0;
    requestOpenAiCompatible(conn.apiUrl, conn.apiKey, model);
}

namespace {

bool shouldRejectSpaceWord(const QString& word, const QSet<QString>& usedInSession) {
    if (word.isEmpty()) {
        return true;
    }
    const QString k = WordProvider::normalizeWord(word);
    const QString key = k.isEmpty() ? word.trimmed().toLower() : k;
    if (key.isEmpty()) {
        return true;
    }
    if (key == QStringLiteral("asteroid")) {
        return true;
    }
    if (usedInSession.contains(key)) {
        return true;
    }
    return false;
}

} // namespace

void WordProvider::requestOpenAiCompatible(const QString& apiUrl, const QString& apiKey, const QString& model) {
    QNetworkRequest req{QUrl(apiUrl)};
    req.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    req.setRawHeader("Authorization", (QStringLiteral("Bearer ") + apiKey).toUtf8());

    QJsonObject userMsg;
    userMsg[QStringLiteral("role")] = QStringLiteral("user");
    const quint64 nonce = QRandomGenerator::global()->generate64();
    QString userContent = QStringLiteral(
        "Generate one English word for a space-themed typing minigame. "
        "Requirements: 6-12 letters, only lowercase a-z, common dictionary word related to space, astronomy, "
        "rocketry, spacecraft, or orbital mechanics. "
        "Hard rule: the word must NOT be \"asteroid\". Pick a noun never used before in this session. ");
    if (!m_usedApiChrono.isEmpty()) {
        constexpr int kMaxListed = 48;
        const QStringList tail = m_usedApiChrono.mid(qMax(0, m_usedApiChrono.size() - kMaxListed));
        userContent += QStringLiteral("Do not repeat any of these words already used: %1. ")
                           .arg(tail.join(QStringLiteral(", ")));
    }
    if (m_wordRequestAttempt > 0) {
        userContent += QStringLiteral(
            "(Retry — avoid clichés like asteroid/nebula/orbit; prefer e.g. capsule, thruster, module, zenith, "
            "horizon, docking, apogee, beacon, shuttle, payload.) ");
    }
    userContent += QStringLiteral(
        "Diversity token (ignore in output): %1. "
        "Reply with nothing except a single JSON object on one line, exactly: {\"word\":\"yourwordhere\"}.")
                         .arg(QString::number(nonce));
    userMsg[QStringLiteral("content")] = userContent;

    QJsonArray messages;
    QJsonObject sysMsg;
    sysMsg[QStringLiteral("role")] = QStringLiteral("system");
    sysMsg[QStringLiteral("content")] = QStringLiteral(
        "You only output the requested JSON line. No markdown, no code fences, no explanation. "
        "Never choose the word \"asteroid\".");
    messages.append(sysMsg);
    messages.append(userMsg);

    QJsonObject body;
    body[QStringLiteral("model")] = model;
    body[QStringLiteral("messages")] = messages;
    body[QStringLiteral("temperature")] = 1.0;
    body[QStringLiteral("top_p")] = 0.95;
    body[QStringLiteral("max_tokens")] = 64;

    QNetworkReply* reply = m_net->post(req, QJsonDocument(body).toJson(QJsonDocument::Compact));
    QObject::connect(reply, &QNetworkReply::finished, this, [this, reply, apiUrl, apiKey, model]() {
        reply->deleteLater();
        const QByteArray raw = reply->readAll();
        if (reply->error() != QNetworkReply::NoError) {
            llmDiag("network", reply->errorString());
            m_wordRequestAttempt = 0;
            QString w = pickLocalWordAvoidingUsed();
            rememberShownWord(w);
            emit wordReady(w, false);
            return;
        }
        const QVariant httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
        if (httpStatus.isValid() && httpStatus.toInt() >= 400) {
            llmDiag("http", QString::number(httpStatus.toInt()) + QLatin1Char(' ') + QString::fromUtf8(raw.left(500)));
            m_wordRequestAttempt = 0;
            QString w = pickLocalWordAvoidingUsed();
            rememberShownWord(w);
            emit wordReady(w, false);
            return;
        }
        QString word = parseWordFromChatCompletionJson(raw);
        if (word.isEmpty()) {
            const QJsonDocument root = QJsonDocument::fromJson(raw);
            if (root.isObject() && root.object().contains(QStringLiteral("error"))) {
                const QJsonObject err = root.object().value(QStringLiteral("error")).toObject();
                llmDiag("api", err.value(QStringLiteral("message")).toString());
            } else {
                llmDiag("parse", QStringLiteral("no word in response; preview=") + QString::fromUtf8(raw.left(700)));
            }
            m_wordRequestAttempt = 0;
            QString w = pickLocalWordAvoidingUsed();
            rememberShownWord(w);
            emit wordReady(w, false);
            return;
        }
        if (shouldRejectSpaceWord(word, m_usedApiWords)) {
            llmDiag("repeat", QStringLiteral("rejected repeated/cliché word: ") + word);
            if (m_wordRequestAttempt < 5) {
                ++m_wordRequestAttempt;
                requestOpenAiCompatible(apiUrl, apiKey, model);
                return;
            }
            m_wordRequestAttempt = 0;
            QString w = pickLocalWordAvoidingUsed();
            rememberShownWord(w);
            emit wordReady(w, false);
            return;
        }
        m_wordRequestAttempt = 0;
        rememberShownWord(word);
        emit wordReady(word, true);
    });
}
