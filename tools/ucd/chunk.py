
import sys

class chunk (object):
    def __init__(self, chunk_id, descriptions):
        self.chunk_id = chunk_id
        self.descriptions = descriptions

    def __eq__(self, other):
        return self.descriptions == other.descriptions

def make_chunk(descriptions, chunk_id, chunk_size):
    i = chunk_id * chunk_size
    return chunk(chunk_id, descriptions[i:i + chunk_size])

def deduplicate_chunks(descriptions, chunk_size = 32):
    """Deduplicate chunks of description data.

    The descriptions are first chunked in groups of @a chunk_size.
    Then for each chunk a check is done if a previous chunk has the same data, if it does the chunk
    is thrown away.

    The index table contains contains `len(descriptions) / chunk_size` entries. Each entry contains
    the index in the returned chunks after deduplication.

    @param descriptions All the unicode-descriptions
    @param chunk_size The number of unicode-descriptions in a chunk, typically 32.
    @return index-table, chunk-table.
    """

    assert(len(descriptions) % chunk_size == 0)
    num_chunks = len(descriptions) // chunk_size

    print("Deduplicating {} chunks, {} code-points.".format(num_chunks, len(descriptions)), file=sys.stderr, flush=True)

    chunks = []
    indices = []
    chunk_index = 0
    for chunk_id in range(num_chunks):
        new_chunk = make_chunk(descriptions, chunk_id, chunk_size)

        # Search for chunks and deduplicate, and keep track of the index where the chunk was found/added.
        try:
            i = chunks.index(new_chunk)
            indices.append(i)
        except ValueError:
            indices.append(len(chunks))
            chunks.append(new_chunk)

    print("Deduplicating done, compressed to {} chunks, {} code-points.".format(
        len(chunks), len(chunks) * chunk_size), file=sys.stderr, flush=True)

    return indices, chunks
