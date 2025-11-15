#include "ModuleManager.h"

namespace Aleng::StdLib {
    NativeLibrary CreateMathLibrary();
    NativeLibrary CreateTestLibrary();
}

namespace Aleng {
    void RegisterAllNativeLibraries(ModuleManager& manager)
    {
        manager.RegisterNativeLibrary("std/math", StdLib::CreateMathLibrary());
        manager.RegisterNativeLibrary("std/test", StdLib::CreateTestLibrary());
    }
}