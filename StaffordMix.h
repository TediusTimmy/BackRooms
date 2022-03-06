/*
   No rights applicable.
*/

/*
   This is David Stafford's variant 13 on improving the finalizer for
   MurmurHash3. This is used in Java's SplittableRandom class.
   https://zimbry.blogspot.com/2011/09/better-bit-mixing-improving-on.html

   Here, it is used to "improve" or "harden" a very-low-entropy random source,
   a use Stafford recommends for the function.
*/

uint64_t harden (uint64_t z)
 {
   z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9UL;
   z = (z ^ (z >> 27)) * 0x94d049bb133111ebUL;
   return z ^ (z >> 31);
 }
