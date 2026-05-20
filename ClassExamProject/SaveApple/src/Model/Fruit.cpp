#include "Fruit.h"

Fruit::Fruit(FruitType type, QPointF position, qreal speed)
    : m_type(type), m_position(position), m_speed(speed), m_smashFrames(0), m_letter('?') {}

FruitType Fruit::type() const {
    return m_type;
}

void Fruit::setType(FruitType type) {
    m_type = type;
}

QPointF Fruit::position() const {
    return m_position;
}

void Fruit::setPosition(const QPointF& position) {
    m_position = position;
}

qreal Fruit::speed() const {
    return m_speed;
}

void Fruit::setSpeed(qreal speed) {
    m_speed = speed;
}

int Fruit::smashFrames() const {
    return m_smashFrames;
}

void Fruit::setSmashFrames(int frames) {
    m_smashFrames = frames;
}

void Fruit::tickSmashFrame() {
    if (m_smashFrames > 0) {
        --m_smashFrames;
    }
}

QChar Fruit::letter() const {
    return m_letter;
}

void Fruit::setLetter(QChar letter) {
    m_letter = letter;
}

void Fruit::update(float deltaTime) {
    m_position.setY(m_position.y() + m_speed * deltaTime);
}

QRectF Fruit::bounds(const QSize& spriteSize) const {
    return QRectF(
        m_position.x() - spriteSize.width() * 0.5,
        m_position.y() - spriteSize.height() * 0.5,
        spriteSize.width(),
        spriteSize.height());
}

int Fruit::scoreValue() const {
    if (m_type == FruitType::GoodApple) {
        return 10;
    }
    if (m_type == FruitType::SmallApple) {
        return 5;
    }
    return 0;
}
