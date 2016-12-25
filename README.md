# mtimeout - kill a process if it doesn't touch a file

`mtimeout` runs a process with a timeout and a file:

    mtimeout /tmp/foo 60 sh -c 'while true; touch /tmp/foo; done'

In the above example, if `/tmp/foo` is not modified for a span of more
than 60 seconds, it will be killed.

## Building

    make
    ./mtimeout ...

## Implementation

Every second, `mtimeout` will read the given file / directory's
metadata. If the file's modification ("mtime") doesn't change for a span
of <timeout> seconds, the child process will be killed.

If the child process exits normally, `mtimeout` will exit with the same
status code.

## Similarity to "timeout"

`timeout` is an existing GNU coreutils process. It kills a process if it
takes longer than a given timeout:

    timeout 5 sleep 10
    # killed after 5 seconds


This is great for processes that should only run for a short period:
`timeout 60 curl`, etc.

But `timeout` is inappropriate for "daemons", processes that are
supposed to run forever. `timeout 60 nginx` is a bad idea.

Instead, you can use `mtimeout` when a process ought to produce or
modify a file on a regular basis. This acts as a basic "watchdog" for a
process. A common setup may be `while true; mtimeout ... ; done`.
