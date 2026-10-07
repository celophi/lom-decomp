"""Exact slot weights for field_roll_defeat_drop, conditional on level and flags."""

from fractions import Fraction


EXTRA_SLOTS = 0x04000000
NO_COMMON = 0x08000000
SCENARIOS = (("normal", 0), ("extra_slots", EXTRA_SLOTS),
             ("no_common", NO_COMMON), ("both", EXTRA_SLOTS | NO_COMMON))


def slot_weights(boundary: int, flags: int = 0) -> list[int]:
    """Count the 128 equally weighted low-seven-bit masks selecting each slot.

    The table value is the final fallback slot, not the number of choices.
    Earlier slots win on a set bit; the fallback also receives all-zero masks.
    """
    if not 0 <= boundary <= 255:
        raise ValueError("drop boundary must be an unsigned byte")
    last = min(boundary + (2 if flags & EXTRA_SLOTS else 0), 7)
    weights = []
    remaining = 128
    for slot in range(last):
        weight = 0 if flags & NO_COMMON and slot < 2 else remaining // 2
        weights.append(weight)
        remaining -= weight
    return weights + [remaining] + [0] * (7 - last)


def profiles(boundaries: tuple[int, ...]) -> list[dict]:
    if len(boundaries) != 8:
        raise ValueError("the drop level table must contain eight bytes")
    result = []
    for name, flags in SCENARIOS:
        bands = []
        for band, boundary in enumerate(boundaries):
            minimum, maximum = max(1, band * 16), min(99, band * 16 + 15)
            if minimum > maximum:
                continue
            bands.append({"level_min": minimum, "level_max": maximum,
                          "table_index": band, "last_slot": min(boundary + (2 if flags & EXTRA_SLOTS else 0), 7),
                          "weights_out_of_128": slot_weights(boundary, flags)})
        result.append({"name": name, "defeat_flags": flags, "bands": bands})
    return result


def choice_rates(slots: list[int], profile: dict) -> list[dict]:
    """Combine repeated drop choices and adjacent levels with the same chance."""
    result = []
    for band in profile["bands"]:
        weight = sum(band["weights_out_of_128"][slot] for slot in slots)
        if result and result[-1]["weight_out_of_128"] == weight:
            result[-1]["level_max"] = band["level_max"]
        else:
            result.append({"level_min": band["level_min"], "level_max": band["level_max"],
                           "weight_out_of_128": weight, "fraction": str(Fraction(weight, 128)),
                           "percent": f"{weight * 100 / 128:g}%"})
    return result


def format_rates(rates: list[dict]) -> str:
    return "; ".join(f"Lv {rate['level_min']}-{rate['level_max']}: {rate['percent']}" for rate in rates)
