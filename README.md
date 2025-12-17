# Exposure Controller

## Usage

Once compiled, usage takes the following form:

```
./exposure_controller -d /dev/video0 -e 100
```

Where `-d` specifies the video device and `-e` specifies the desired exposure value.

## Compilation

Nothing special.
Just run:

```
gcc exposure_controller.c -o exposure_controller
```

## Prerequisites

Everything was included from default on a Pi 4 running Raspberry Pi OS 64-bit.
Who knows what it actually needs :confused:

## References and Links

[Kernel Docs](https://docs.kernel.org/userspace-api/media/v4l/ext-ctrls-camera.html) was pretty helpful and had a decent reference for the exact control IDs.
