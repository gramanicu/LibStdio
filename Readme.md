# SO_STDIO

A simple C i/o library, the second homework for the Operating Systems Course. The problem statement can be found [here](https://ocw.cs.pub.ro/courses/so/teme/tema-2) (in romanian)

## Organization

There is only one ".c" file that I written myself, "so_stdio.c". It contains the almost all the implementations for the functions declared in the library header (no processes). I also used two headers that I written for the previous homework.

## Implementation

Most of the functionalities are implemented (except the processes), but some of them have some bugs (there were even more, but they were removed). The code was implemented as simply as I could, as I was time-constrained.

The structure that contains the date used by the streams was implemented on the go: I added properties as they were needed:

- _buf - the buffer that holds data read/wrote (i/o buffering)
- _fd - the file descriptor
- _flags - the flags used to open the file
- _position - the current "index" in the buffer
- _last_op - the last operation (OP_READ / OP_WRITE)
- _last_read - how many bytes were read into the buffer the last time
- _total_bytes - how many bytes are stored in the buffer
- _eof - the "End Of File" flag
- -err - the "Error" flag

There are a few simple functions, that only return some data from the stream (properties). The more complex functions are:

- so_fopen - opens a file (and creates it, if needed), with different flags. This also initializes the stream.
- so_fclose - close the file. Flushes the stream.
- so_fflush - removes all data from the buffer. If the last operation was a OP_WRITE, the data is written to the output file.
- so_fseek - "seeks" through the file. Will flush the stream.
- so_fgetc - returns a char from the file. If the buffer is empty (no data read or all the chars from it were returned), the buffer will be filled (as much as possible) with data from the file.
- so_fputc - writes a char to the buffer.
- so_fread - reads the specified amount of data (1 by 1, using fgetc). Initially, this was implemented on its own, but it was very hard to debug.
- so_fwrite - write the specified amount of data to the buffer (using fputc)

## Compilation

- Linux: `make build`
- Windows: NA

## Sources

- [OS Laboratory](https://ocw.cs.pub.ro/courses/so)
- [Make template](https://gist.github.com/keeferrourke/fe72476a8dd8c4c02ff18eaed74e1de0)
