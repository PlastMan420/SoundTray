@import "cpp-style-google.md"

# HttpSrv Agent Guidelines

This repository contains a Windows C++ HTTP server. Keep each agent focused on
one responsibility and consistent with the surrounding code.

## Agent Structure

- Declare public APIs in headers and implement them in source files.
- Use `AgentInterface` for shared agent contracts when an interface is needed.
- Use `RequestAgent` for request parsing and dispatch.
- Use `ResponseAgent` for response construction.
- Use `HealthAgent` for health checks.

## Windows Conventions

- Prefer `std::string` and `std::wstring` for string ownership. Use
	`std::wstring` when an API requires wide Windows strings.
- Use existing Windows typedefs such as `PCHAR` and `PCWSTR` at Windows API
	boundaries.
- Follow the repository's existing exception and resource-management behavior
	when integrating with Windows APIs.

## File Names

- Headers: `agent_name.h`
- Sources: `agent_name.cc`
- Tests: `agent_name_test.cc`

When this file conflicts with the imported guide, the repository-specific rules
above take precedence. Otherwise, follow `cpp-style-google.md` and preserve
the established style of nearby code.
