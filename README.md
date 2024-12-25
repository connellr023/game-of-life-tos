# Game of Life $(\text{Transient OS})$

> _Conway's Game of Life_ simulated on $\text{Transient OS}$ threads.

![Raspberry Pi](https://img.shields.io/badge/-Raspberry_Pi-C51A4A?style=for-the-badge&logo=Raspberry-Pi)
![C++](https://img.shields.io/badge/c++-%2300599C.svg?style=for-the-badge&logo=cplusplus&logoColor=white)
![ARMv8](https://img.shields.io/badge/ARMv8-%23000000.svg?style=for-the-badge&logo=arm&logoColor=00c1de)

## Overview

This project is a simulation of _Conway's Game of Life_ on a Raspberry Pi using **257** threads. The threads are created using `libtransient-os` and the game is displayed through the framebuffer.

`libtransient-os` or $\text{Transient OS}$ is a time-sharing operating system I developed that supplies an API for scheduling threads in a bare-mental environment for the Raspberry Pi 3. See the main repository [here](https://github.com/connellr023/transient-os).

Additionally, drivers (`librpi3-drivers`) for the framebuffer, UART, and system timer can be found at a seperate repository I created [here](https://github.com/connellr023/rpi3-drivers).

## Multi-threading

Each cell in the grid is assigned to a thread. Additionally, there is another thread that is responsible for swapping the grids. Since the grid is $16 \times 16$, there are a total of $257$ cell threads and $1$ swap thread.
