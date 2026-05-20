/* -------------------------------------------------------------------------
//  文件名    : WordProvider.h
//  创建者    : dengzihang
//  创建时间  : 2026-05-04
//  功能描述  : 奖励词提供器——LLM API + 本地词库回退
// -------------------------------------------------------------------------*/

#pragma once

#include <QObject>
#include <QSet>
#include <QString>
#include <QStringList>

class QNetworkAccessManager;

/// @file WordProvider.h
/// 奖励关英文单词：通过 OpenAI 兼容 Chat Completions 请求大模型；未配置或失败时回退内置本地词库。
///
/// 配置：可执行文件同目录 llm_config.json（字段 apiUrl、apiKey、model），优先于环境变量。
/// 可复制 llm_config.example.json（通用）或 llm_config.aliyun.example.json（阿里云百炼）为 llm_config.json。
///
/// 环境变量补全：SPACE_LLM_API_*；阿里云可与 Go 示例一致使用 DASHSCOPE_API_KEY（Bearer）。
/// 可选 DASHSCOPE_COMPATIBLE_URL（默认不写 URL 且仅用百炼 Key 时，使用北京地域兼容端点：
/// https://dashscope.aliyuncs.com/compatible-mode/v1/chat/completions ）；新加坡地域请自行设置 URL。
/// DASHSCOPE_MODEL 可补全 model；请求百炼且 model 仍为空时默认 qwen-plus。
class WordProvider : public QObject {
    Q_OBJECT
public:
    explicit WordProvider(QObject* parent = nullptr);

    /// 异步获取一个单词；成功时 fromApi=true（大模型），否则为本地词库。
    void requestWord();

    /// 清理字符串：去除非字母、小写化，要求 4-24 字符。
    static QString normalizeWord(const QString& raw);

signals:
    void wordReady(const QString& word, bool fromApi);

private:
    QString pickLocalWord(); ///< 轮询内置词表。
    /// 优先返回尚未在本次运行中出现过的本地词（与大模型去重用同一套 m_usedApiWords）。
    QString pickLocalWordAvoidingUsed();
    void rememberShownWord(const QString& word); ///< 记录已出示的词（小写规范键），供去重与 prompt。
    /// POST JSON 到兼容接口；若返回禁用/本会话已用词会自动重试至多 5 次。
    void requestOpenAiCompatible(const QString& apiUrl, const QString& apiKey, const QString& model);

    QNetworkAccessManager* m_net = nullptr;
    QStringList m_localWords;
    int m_localIndex = 0;
    QSet<QString> m_usedApiWords; ///< 本进程内已成功出示过的词（小写），大模型与本地均不得重复
    QStringList m_usedApiChrono;  ///< 顺序列表，用于拼进 prompt 的「近期已用」摘要（可长于 used 仅摘尾部）
    int m_wordRequestAttempt = 0; ///< 当前这一轮请求内的重试次数（requestWord 时清零）
};
