# ReactorHttp

一个基于 C++ 实现的 Reactor 模式高性能 HTTP 服务器/学习项目。

## 📖 项目简介

本项目主要用于学习和实践 C++ 高级编程技术，核心目标是构建一个基于 **Reactor** 事件驱动模型的高性能 HTTP 服务器。

通过实现该项目，你可以深入理解和掌握：
- **现代 C++** 特性（如智能指针、移动语义、lambda 表达式等）在实际项目中的应用。
- **网络编程**：TCP/IP 协议栈、Socket 编程、HTTP 协议解析与构建。
- **并发编程**：多线程、线程池、事件循环、锁机制，以及如何避免竞态条件。
- **设计模式**：Reactor/Proactor 模式、工厂模式、单例模式等在网络框架中的应用。
- **性能优化**：I/O 多路复用（epoll）、缓冲区设计、定时器管理等。

## 🏗️ 项目架构与核心技术栈

### 核心模型
- **Reactor 模式**：主从 Reactor 多线程模型，主线程负责接受连接，多个工作线程负责处理 I/O 事件和业务逻辑。
- **非阻塞 I/O**：使用 `epoll`（Linux）实现 I/O 多路复用，实现高并发连接管理。

### 技术栈
- **语言**: C++17/20
- **网络库**: Linux epoll / (可扩展支持 Windows IOCP)
- **并发**：`std::thread`, `std::mutex`, `std::condition_variable`, 线程池
- **HTTP解析**: 手写 HTTP Request/Response 解析器（状态机）
- **工具链**: CMake, GCC/Clang, GTest (单元测试)

## 📁 项目结构
