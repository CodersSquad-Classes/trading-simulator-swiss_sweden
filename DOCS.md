# 1. Introduction

This project implements a Continuous Limit Order Book (CLOB) in C++, simulating how modern electronic exchanges match buy and sell orders in real time.
The system continuously generates random buy and sell orders, processes them through a matching engine, and displays a real-time terminal dashboard that updates in place without flicker—similar to a professional trading terminal.

The project highlights:

- Priority queue–based order handling

- Price–time priority matching logic

- Real-time, in-place terminal UI

- Multithreaded simulation (order generation + UI refresh)

- Clean separation between order book logic and rendering

Use of modern C++ features (C++17)

# 2. Research on Stock Exchanges

Modern stock exchanges such as NASDAQ, NYSE, and most crypto exchanges use a Continuous Limit Order Book as their core mechanism for determining market prices.

How a CLOB Works

A CLOB stores all open buy and sell limit orders:

- Buy (bid) orders sorted by highest price first, then earliest timestamp

- Sell (ask) orders sorted by lowest price first, then earliest timestamp

When a new order arrives, it is matched against the best available opposing orders:

- A buy order matches with the lowest ask

- A sell order matches with the highest bid

This ensures:

- Price priority: Better prices get executed first.

- Time priority: Earlier orders at the same price are executed first.

- Transparency: Anyone can see market depth (best prices + volumes).

Fair execution: No trader is given unfair priority.

CLOBs enable:

- High-frequency trading

- Real-time price discovery

- Transparency into market depth

- Continuous matching (millisecond-level)

Our implementation is inspired by real-world systems like the Island ECN, one of the earliest and most influential electronic limit order books.

