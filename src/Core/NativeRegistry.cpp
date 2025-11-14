#include "ModuleManager.h"

namespace Aleng::StdLib {
    NativeLibrary CreateMathLibrary();
}

namespace Aleng {
    void RegisterAllNativeLibraries(ModuleManager& manager)
    {
        manager.RegisterNativeLibrary("std/math", StdLib::CreateMathLibrary());
    }
}