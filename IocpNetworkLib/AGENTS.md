# IOCP Network Library - Agent Guidelines

## Build Commands
- **Build solution**: `msbuild IocpNetworkLib.sln /p:Configuration=Debug /p:Platform=x64`
- **Build release**: `msbuild IocpNetworkLib.sln /p:Configuration=Release /p:Platform=x64`
- **Run test server**: `bin\x64\Debug\TestServer.exe` (after building)
- **Run dummy client**: `bin\x64\Debug\DummyClient.exe` (after building)

## Code Style Guidelines
- **Language**: Modern C++23 with UTF-8 encoding (`/utf-8` flag)
- **Naming**: PascalCase for classes/methods, camelCase for variables, underscore prefix for private members (`_member`)
- **Namespace**: All code in `csmnet` namespace, organized by domain (`network`, `detail`, `util`, `dto`, `common`)
- **Headers**: Use `#pragma once`, group includes (project headers first, then standard library)
- **Types**: Use custom type aliases from `Type.h` (`int32`, `uint64`, etc.) instead of built-in types
- **Error handling**: Use `std::expected<T, error_code>` for fallible operations, custom error enums for domain errors
- **Memory**: RAII patterns, smart pointers, avoid raw pointers
- **PCH**: Include `pch.h` first in all `.cpp` files for precompiled headers
- **Formatting**: Space indentation (editorconfig), UTF-8 charset
- **Dependencies**: spdlog for logging, vcpkg for package management
- **Testing**: Use TestServer/DummyClient projects for integration testing