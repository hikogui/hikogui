
import sys

class Chunk (list):
    def __hash__(self):
        r = 0
        for x in self:
            r ^= hash(x)
        return r


def deduplicate_by_chunk_size(src, chunk_size):
    """
    The input is chunked in 32 items per chunk.
    Then chunks are compared to each other and deduplicated.

    @param src A list of items one item for each of the 0x110000 unicode code-points.
               Items should be equality comparable and is hashable
    @param chunk_size The size of a chunk.
    @return deduplicated data, index_table.
    """

    assert(len(src) == 0x110000)

    chunks = {}
    for index, offset in enumerate(range(0, len(src), chunk_size)):
        chunk = Chunk(src[offset:offset + chunk_size])
        indices = chunks.setdefault(chunk, [])
        indices.append(index)

    # Sort the chunks by the first index.
    chunks_items = list(chunks.items())
    chunks_items.sort(key = lambda x: x[1][0])

    # Build the chunk index table.
    chunk_indices = [0] * (0x110000 // chunk_size)
    # At the same time build the deduplicated data.
    dst = []
    for chunk_offset, (chunk, indices) in enumerate(chunks_items):
        dst += chunk
        for index in indices:
            chunk_indices[index] = chunk_offset

    # Strip off repeated indices at the end.
    last_index = chunk_indices[-1]
    for i in range(len(chunk_indices) - 1, -1, -1):
        if chunk_indices[i] != last_index:
            chunk_indices = chunk_indices[:i + 2]
            break

    return dst, chunk_indices


def deduplicate(src):
    """
    The input is chunked in 32 items per chunk.
    Then chunks are compared to each other and deduplicated.

    @param src A list of items one item for each of the 0x110000 unicode code-points.
               Items should be equality comparable and is hashable
    @param chunk_size The size of a chunk.
    @return deduplicated data, index_table.
    """

    best_score = None
    best_dst = None
    best_indices = None
    best_chunk_size = None
    for i in range(16):
        chunk_size = 1 << i

        dst, indices = deduplicate_by_chunk_size(src, chunk_size)
        dst_width = max(x.bit_length() for x in dst)
        index_width = max(x.bit_length() for x in indices)

        dst_size = len(dst) * dst_width
        indices_size = len(indices) * index_width
        size = dst_size + indices_size

        # Index width of 8 is much more performant, give it a better score.
        score = int(size * 0.9) if index_width == 8 else size
        if best_score is None or score < best_score:
            best_score = score
            best_dst = dst
            best_indices = indices
            best_chunk_size = chunk_size

    return best_dst, best_indices, best_chunk_size



