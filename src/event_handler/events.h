#pragma once

enum class Event {
    CloseWindow,
    ResizedWindow,
    ToggleFullscreen,
    ToggleHelp,
    ToggleUIVisibility,
    ScrollLeft,
    ScrollRight,
    ScrollUp,
    ScrollDown,
    ZoomIn,
    ZoomOut,
    CalculateImage,
    ColorizeImage,
    ChangeNumberOfThreads,
    CancelCalculation,
};
