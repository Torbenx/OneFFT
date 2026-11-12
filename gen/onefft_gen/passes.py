"""Loop structure shared by the pass kernels of all targets.

A middle pass works on `regions` regions of `blocks` blocks of `inner` butterflies:

    for each region:                    twiddle table: the next args.tw_region bytes
        for each block:                 the twiddle table starts over
            for each butterfly: ...
            pointers += args.skip
        pointers += args.region_skip

The numbers that do not change between calls are read from a struct, `onefft_middle_args` in
onefft.c, whose fields are all 8 bytes and listed in ARGS in order.
"""

ARGS = ("s_high", "s_low", "inner", "skip", "region_skip", "tw_region")


def field(name):
    """Byte offset of a field of onefft_middle_args."""
    return 8 * ARGS.index(name)


def middle_params(k):
    """Declares the parameters every middle pass kernel has."""
    data = k.param("float *", "data")
    tw = k.param("const float *", "tw")
    args = k.param("const size_t *", "args")
    blocks = k.param("size_t", "blocks")
    regions = k.param("size_t", "regions")
    return data, tw, args, blocks, regions


def middle_loops(k, pointers, tw_base, args, blocks, regions, inner_loops):
    """Emits the three loops around the butterflies of a middle pass.

    `pointers` are the registers that walk through the data, `inner_loops(tw, j, scratch)`
    emits the loop(s) over the butterflies of one block: `tw` is the twiddle pointer, `j` a free
    counter register and `scratch` another free register.
    """
    tw = k.gtemp("twp", "const float *")
    j = k.gtemp("j")
    b = k.gtemp("b")
    scratch = k.gtemp("scratch")
    region_top = k.label()
    k.gmov(b, blocks)
    block_top = k.label()
    k.gmov(tw, tw_base)
    inner_loops(tw, j, scratch)
    k.gload(scratch, args, field("skip"))
    for p in pointers:
        k.gadd(p, p, scratch)
    k.loop(b, block_top)
    k.gload(scratch, args, field("region_skip"))
    for p in pointers:
        k.gadd(p, p, scratch)
    k.gload(scratch, args, field("tw_region"))
    k.gadd(tw_base, tw_base, scratch)
    k.loop(regions, region_top)


def last_loops(k, pointers, count, regions, region_skip, inner_loops):
    """Emits the loop over the regions of a last pass: `inner_loops(j)` handles `count`
    butterflies with the free counter register j; then the pointers move on by `region_skip`."""
    j = k.gtemp("j")
    region_top = k.label()
    inner_loops(j)
    for p in pointers:
        k.gadd(p, p, region_skip)
    k.loop(regions, region_top)
