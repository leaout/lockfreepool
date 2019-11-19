# lfthreadpool

lfthreadpool is a lock-free thread pool, it helps process tasks quickly.

## Features

using lock-free queue to avoid multithreading competing for a single queue.
Task scheduling uses rotation training and minimum load.

how it works

![lockfree](lockfree.png)

## compile

### linux:

cmake  .

make 

### windows:

use cmake or  open testLFTPool.sln directly.

