#include "evaluation.h"

Evaluator::Evaluator() : fps(0.0), processTimeMs(0.0) {}

void Evaluator::start() {
    tm.reset();
    tm.start();
}

void Evaluator::stop() {
    tm.stop();
    processTimeMs = tm.getTimeMilli();
    // 限制最小耗时避免除零错误
    if (processTimeMs > 0) {
        fps = 1000.0 / processTimeMs;
    }
}

double Evaluator::getFPS() const { return fps; }
double Evaluator::getTime() const { return processTimeMs; }
