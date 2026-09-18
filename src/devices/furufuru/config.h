#pragma once

// 機体表示名
#define DEVICE_NAME "フルフル"

// 有効タスク一覧（将来、機体ごとに有効タスクを変えるための拡張ポイント）
static const char* const DEVICE_TASK_LIST[] = {
    "task1", "task2", "task3", "task4", "task5", "option"
};
static constexpr uint8_t DEVICE_TASK_COUNT = 6;
