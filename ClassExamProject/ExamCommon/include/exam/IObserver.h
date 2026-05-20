/* -------------------------------------------------------------------------
//  文件名    : IObserver.h
//  创建者    : dengzihang
//  创建时间  : 2026-05-04
//  功能描述  : 观察者接口——Model 数据变化时通知 View
// -------------------------------------------------------------------------*/

#pragma once

// Shared observer interface for ClassExam games (SaveApple GameData; SpaceBattle may adopt).

class IObserver {
public:
    virtual ~IObserver() = default;
    virtual void onModelChanged() = 0;
};
