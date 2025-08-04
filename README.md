# Multithreaded Elevator Simulation

## Overview
This project simulates a real-time elevator control system implemented in C using POSIX threads (pthreads). Built as a CS 361 project at the University of Illinois Chicago, this project models multiple elevators and passengers, managing synchronization and coordination between them.

## Features
- Real-time simulation of elevator movement and passenger interactions
- Thread-safe shared data structures protected by mutexes and condition variables
- State machines to coordinate elevator doors, movement, and passenger states
- Efficient load balancing across elevators using request scheduling
- Uses `sched_yield()` and trip tracking for smooth operation

## Technologies
- C programming language
- POSIX Threads (pthreads) for multithreading and synchronization
- `ncurses` library for terminal control (if applicable)
- GitHub and GitHub Copilot for version control and development assistance

## Building the Project
This project includes a `Makefile` for easy compilation.

To build the project, simply run:

```bash
make
