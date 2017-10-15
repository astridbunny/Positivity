# Positivity
Positivity is a Node.js module intended to both (sort of) replicate [Python's Random module](https://docs.python.org/3/library/random.html) and implement [ISAAC](http://burtleburtle.net/bob/rand/isaacafa.html) for cryptographic security and speed. Currently a work in progress, and should not be used in production.

## Why use Positivity?
### Security
Positivity implements the [ISAAC](http://burtleburtle.net/bob/rand/isaacafa.html) pseudo-random number generator, which [is cryptographically secure](https://crypto.stackexchange.com/questions/15976/is-isaac-cipher-cryptographically-secure). Seeds are pulled from system-specific sources; /dev/urandom on UNIX-based systems and CryptGenRandom on windows.

### Speed
When tested with [Benchmark.js](https://benchmarkjs.com/), Positivity has an advantage of several hundred thousand more operations per second over the crypto module.

## Installation
Build, require, and then `positivity.Initialize()`

## API
All methods (except for Initialize) have an optional callback parameter. If this parameter is not included, methods are not guaranteed to be asynchronous.

### `positivity.Initialize()`
Initializes the random number generator. Should be called before any functions are used.

### `positivity.getInt()`
Returns a random integer between -2147483647 and 2147483647.

### `positivity.getFloat()`
Returns a random floating point number between -1 and 1.

### `positivity.getIntInRange(min, max)`
Returns a random integer between `min` and `max`.

### `positivity.getFloatInRange(min, max)`
Returns a random floating point number between `min` and `max`.

### `positivity.Choice(arr)`
Returns a random element from the array `arr`.

### `positivity.Choices(arr, size)`
Returns `size` random samples from array `arr`.

### `positivity.Shuffle(arr)`
Returns `arr` after being shuffled.

### `positivity.Bytes(length)`
Returns a buffer with `length` random bytes.
