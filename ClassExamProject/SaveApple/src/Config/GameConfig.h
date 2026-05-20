/* -------------------------------------------------------------------------
//  文件名    : GameConfig.h
//  创建者    : dengzihang
//  创建时间  : 2026-05-04
//  功能描述  : SaveApple 配置单例——资源路径、游戏参数常量
// -------------------------------------------------------------------------*/

#pragma once

#include <QString>

class GameConfig {
public:
    // 单例入口：全局唯一配置对象。
    static GameConfig& instance();

    // 初始生命值（新开局/重置时使用）。
    int initialLives() const;
    // 生成水果的时间间隔（毫秒）。
    int spawnIntervalMs() const;
    // 游戏逻辑更新的时间间隔（毫秒）。
    int updateIntervalMs() const;

    // 资源根目录（图片等），通常指向 SaveApple/resources/images。
    QString resourceRoot() const;
    // 获取指定图片文件的实际路径（找不到则返回空字符串）。
    QString imagePath(const QString& filename) const;
    // 公共图片路径接口（目前与 imagePath 相同，保留用于后续区分目录）。
    QString commonImagePath(const QString& filename) const;

private:
    // 私有构造：强制使用单例。
    GameConfig() = default;
};
