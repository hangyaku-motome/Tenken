## file layout as of version 1:
- magic_bytes uint64_t
- file_version uint8_t
- entry_size sizeof(Pointer::Chain) uint8_t
- entry_start_point uint64_t
- filter_index (0 for initial scan, +1 for every filter done) uint8_t

- Module headers.
each entry is:
uint8_t len, then the module name.

- Lastly the chains.


- Important. Offsets are target -> root. Meaning, resolving or displaying them needs to reverse the valid offsets to work properly. (or..walk backwards.)
However, getFrom or rawGetFrom already reverse it before returning chains. Just be mindful when extracting the chains with alternative methods.
