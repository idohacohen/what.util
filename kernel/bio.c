// Buffer cache.
//
// The buffer cache is a linked list of buf structures holding
// cached copies of disk block contents.  Caching disk blocks
// in memory reduces the number of disk reads and also provides
// a synchronization point for disk blocks used by multiple processes.
//
// Interface:
// * To get a buffer for a particular disk block, call bread.
// * After changing buffer data, call bwrite to write it to disk.
// * When done with the buffer, call brelse.
// * Do not use the buffer after calling brelse.
// * Only one process at a time can use a buffer,
//     so do not keep them longer than necessary.


#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "riscv.h"
#include "defs.h"
#include "fs.h"
#include "buf.h"

#define CACHEBUCTS 13

struct {
  struct spinlock lock;
  struct buf buf[NBUF];
  struct spinlock buctlocks[CACHEBUCTS];
  // Linked list of all buffers, through prev/next.
  // Sorted by how recently the buffer was used.
  // head.next is most recent, head.prev is least.
  // struct buf head;
} bcache;

void
binit(void)
{
  struct buf *b;

  initlock(&bcache.lock, "bcache");
  
  struct spinlock *l;
  for (l = bcache.buctlocks; l < bcache.buctlocks + CACHEBUCTS; l++)
  {
    initlock(l, "bcache.bucket");
  }
  

  // Create linked list of buffers
  // bcache.head.prev = &bcache.head;
  // bcache.head.next = &bcache.head;
  for(b = bcache.buf; b < bcache.buf+NBUF; b++){
    // b->next = bcache.head.next;
    // b->prev = &bcache.head;
    initsleeplock(&b->lock, "buffer");
    // bcache.head.next->prev = b;
    // bcache.head.next = b;
  }
}

// Look through buffer cache for block on device dev.
// If not found, allocate a buffer.
// In either case, return locked buffer.
static struct buf*
bget(uint dev, uint blockno)
{
  struct buf *b;

  int bucket = blockno % CACHEBUCTS;
  acquire(&bcache.buctlocks[bucket]);

  // Is the block already cached?
  for(b = bcache.buf; b < bcache.buf + NBUF; b++){
    if(b->dev == dev && b->blockno == blockno){
      b->refcnt++;
      release(&bcache.buctlocks[bucket]);
      acquiresleep(&b->lock);
      return b;
    }
  }
  release(&bcache.buctlocks[bucket]);
  // Not cached.
  // Recycle the least recently used (LRU) unused buffer.
  
  acquire(&bcache.lock);
  acquire(&bcache.buctlocks[bucket]);
  for (int i = bucket; i < bucket + CACHEBUCTS; i++)
  {
    i %= CACHEBUCTS;
    if (i != bucket)
    {
      acquire(&bcache.buctlocks[i]);
    }    
    for(b = bcache.buf; b < bcache.buf + NBUF; b++){
      if(b->refcnt == 0 && b->blockno % CACHEBUCTS == i) {
        b->dev = dev;
        b->blockno = blockno;
        b->valid = 0;
        b->refcnt = 1;
        release(&bcache.buctlocks[bucket]);
        if (i != bucket)
        {
          release(&bcache.buctlocks[i]);
        }
        release(&bcache.lock);
        acquiresleep(&b->lock);
        return b;
      }
      
    }
    if (i != bucket)
    {
      release(&bcache.buctlocks[i]);
    }   
  }
  panic("bget: no buffers");
}


// Return a locked buf with the contents of the indicated block.
struct buf*
bread(uint dev, uint blockno)
{
  struct buf *b;

  b = bget(dev, blockno);
  if(!b->valid) {
    virtio_disk_rw(b, 0);
    b->valid = 1;
  }
  return b;
}

// Write b's contents to disk.  Must be locked.
void
bwrite(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("bwrite");
  virtio_disk_rw(b, 1);
}

// Release a locked buffer.
// Move to the head of the most-recently-used list.
void
brelse(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("brelse");

  releasesleep(&b->lock);

  acquire(&bcache.buctlocks[b->blockno % CACHEBUCTS]);
  b->refcnt--;
  release(&bcache.buctlocks[b->blockno % CACHEBUCTS]);
}

void
bpin(struct buf *b) {
  acquire(&bcache.lock);
  b->refcnt++;
  release(&bcache.lock);
}

void
bunpin(struct buf *b) {
  acquire(&bcache.lock);
  b->refcnt--;
  release(&bcache.lock);
}


