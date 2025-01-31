# chonk

an attempt to break ANY file into smaller chunks, enccrypt it, decrypt the chunks, reassemle and recreate the original file... in c++

## Usage

```bash
$ ./chonk
```

## Example

**Split**

```bash
============================
     FILE SPLITTER CLI
============================
1. Split & Encrypt File
2. Decrypt & Join Files
Enter choice: 1
Enter password (can be empty):
Enter file path: /chonk/s.mp4
Enter chunk size (Mega bytes): 5
Progress: 100.00% (150801346 / 150801346 bytes), Elapsed: 2.36s, Remaining: 0.00s
File split and encrypted into 29 chunks in 's_chunks'.
```

**Join**

```bash
============================
     FILE SPLITTER CLI
============================
1. Split & Encrypt File
2. Decrypt & Join Files
Enter choice: 2
Enter password (can be empty):
Enter chunk directory: s_chunks
Enter output file path: s2.mp4
Progress: 100.00% (29 / 29 chunks), Elapsed: 11.50s, Remaining: 0.00s
File successfully decrypted and reconstructed as 's2.mp4'.
```

## Build from source

```bash
$ git clone git@github.com:ninjaasmoke/chonk.git
$ cd chonk
$ mkdir build
$ cd build
$ cmake ..
$ make
```