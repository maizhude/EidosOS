# EidosOS

![Build](https://img.shields.io/badge/build-passing-brightgreen)
![Status](https://img.shields.io/badge/status-in%20progress-yellow)
![License](https://img.shields.io/badge/license-MIT-blue)
![Language](https://img.shields.io/badge/language-C%20%2F%20Assembly-orange)
![Platform](https://img.shields.io/badge/platform-ARM-lightgrey)

> 一个从零开始构建的实验性操作系统项目，用于探索计算机系统的“本质形态（Eidos）”。

---

## ✨ 项目简介

**EidosOS** 是一个个人操作系统项目，目标是**从底层开始亲手实现一个操作系统**。

这个项目不仅仅是工程实践，更是一次对操作系统“本质”的探索。

项目名称 **Eidos** 源自哲学概念：

> **Eidos = 事物的本质形式 / 理念形态**

本项目希望通过最简洁、最本质的实现方式，理解操作系统的核心抽象。

---

## 🎯 项目目标

* 理解操作系统底层运行机制
* 从零构建一个最小可运行操作系统
* 通过实践掌握核心概念：

  * 任务切换（Task Switching）
  * 同步与互斥（Synchronization & Mutex）
  * 消息机制（Message Passing）
* 保持设计**简洁、清晰、可学习**

---

## 🧠 设计理念

与其一开始构建复杂系统，不如回归本质。

> “操作系统最简单的形态应该是什么？”

EidosOS 的每一个模块都强调：

* 为什么需要它？
* 它的最小实现是什么？
* 它解决的核心问题是什么？

---

## 🛠️ 技术栈

* C 语言 / 汇编
* 裸机开发（Bare Metal）
* CMake + Ninja（构建系统）

---

## 🚧 项目状态

🚧 开发中（从零开始）

---

## 📚 开发路线图

* [X] 从最简单的任务上下文切换开始（保存/恢复寄存器、栈指针）

* [X] 实现基础任务结构（TCB）与任务初始化机制

* [X] 支持任务创建与启动运行（多任务初步运行）

* [X] 实现任务主动让出 CPU（yield）

* [X] 实现时钟中断（定时器驱动）

* [X] 基于时钟中断实现流转调度

* [X] 完善任务切换流程（时钟中断触发切换）

* [X] 扩展调度策略（同优先级流转调度）

* [X] 实现时间片管理与任务状态切换（就绪/运行/阻塞）

* [X] 实现抢占式调度(Systick中断判断由阻塞到就绪的任务优先级是否大于运行的任务优先级)

* [ ] 实现互斥机制（Mutex）

* [ ] 实现同步机制（信号量 Semaphore）

* [ ] 实现事件机制（Event Flags）

* [ ] 实现消息队列（Message Queue）

* [ ] 支持任务间通信（IPC）

* [ ] 编写基础 Bootloader

* [ ] 完成内核初始化流程

* [ ] 实现基础驱动（屏幕输出、串口等）

* [ ] 文件系统基础（简单文件读写）

* [ ] 设备驱动框架（抽象设备接口）

* [ ] 系统调试与日志机制

* [ ] 性能优化与系统稳定性提升

---

## 🤝 贡献

这是一个以学习为主的项目，但欢迎讨论和交流想法。

---

## 📌 说明

本项目不以替代现有操作系统为目标，
而是用于探索操作系统的核心原理与设计本质。

---

> 构建系统，理解本质。
