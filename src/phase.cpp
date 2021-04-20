#include "phase.h"

const char* phase_name(const Phase phase)
{
    switch (phase) {
    case Phase::Starting:
        return "starting";
    case Phase::Idle:
        return "idle";
    case Phase::RequestSent:
        return "request sent";
    case Phase::RequestReceived:
        return "request received";
    case Phase::Waiting:
        return "waiting";
    case Phase::Coloring:
        return "coloring";
    case Phase::Shutdown:
        return "shutdown";
    case Phase::Canceled:
        return "canceled";
    default:
        return "unknown";
    }
}
