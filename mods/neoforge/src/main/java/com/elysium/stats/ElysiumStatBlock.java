package com.elysium.stats;

import com.mojang.serialization.Codec;

import java.util.EnumMap;
import java.util.Map;

/**
 * An immutable bag of stat points.
 *
 * Used for four different things that all want the same shape: a race's
 * starting stats, a race or class's growth per level, the points a player has
 * spent, and the total a piece of gear is granting. Keeping them the same type
 * means adding them together is the only operation the rest of the mod needs.
 *
 * Serialised as a plain string→int map rather than a fixed record, so a save
 * written before a stat existed still loads, and one written after a stat is
 * removed drops the orphan instead of failing.
 */
public final class ElysiumStatBlock {

    public static final ElysiumStatBlock EMPTY = new ElysiumStatBlock(new EnumMap<>(ElysiumStat.class));

    public static final Codec<Map<String, Integer>> MAP_CODEC =
            Codec.unboundedMap(Codec.STRING, Codec.INT);

    private final Map<ElysiumStat, Integer> values;

    private ElysiumStatBlock(Map<ElysiumStat, Integer> values) {
        this.values = values;
    }

    // ------------------------------------------------------------------
    // Construction
    // ------------------------------------------------------------------

    /** Builds from alternating stat/amount pairs — readable at a call site. */
    public static ElysiumStatBlock of(Object... pairs) {
        Map<ElysiumStat, Integer> map = new EnumMap<>(ElysiumStat.class);
        for (int i = 0; i + 1 < pairs.length; i += 2) {
            map.put((ElysiumStat) pairs[i], (Integer) pairs[i + 1]);
        }
        return new ElysiumStatBlock(map);
    }

    public static ElysiumStatBlock fromMap(Map<String, Integer> raw) {
        Map<ElysiumStat, Integer> map = new EnumMap<>(ElysiumStat.class);
        for (Map.Entry<String, Integer> entry : raw.entrySet()) {
            ElysiumStat stat = ElysiumStat.byId(entry.getKey());
            if (stat != null) {
                map.put(stat, entry.getValue());
            }
        }
        return new ElysiumStatBlock(map);
    }

    public Map<String, Integer> toMap() {
        Map<String, Integer> raw = new java.util.LinkedHashMap<>();
        for (Map.Entry<ElysiumStat, Integer> entry : values.entrySet()) {
            if (entry.getValue() != 0) {
                raw.put(entry.getKey().getId(), entry.getValue());
            }
        }
        return raw;
    }

    // ------------------------------------------------------------------
    // Reading and combining
    // ------------------------------------------------------------------

    public int get(ElysiumStat stat) {
        Integer value = values.get(stat);
        return value == null ? 0 : value;
    }

    public boolean isEmpty() {
        for (ElysiumStat stat : ElysiumStat.values()) {
            if (get(stat) != 0) {
                return false;
            }
        }
        return true;
    }

    public ElysiumStatBlock plus(ElysiumStatBlock other) {
        Map<ElysiumStat, Integer> map = new EnumMap<>(ElysiumStat.class);
        for (ElysiumStat stat : ElysiumStat.values()) {
            int sum = get(stat) + other.get(stat);
            if (sum != 0) {
                map.put(stat, sum);
            }
        }
        return new ElysiumStatBlock(map);
    }

    /** Every entry multiplied — how a per-level growth block becomes a total. */
    public ElysiumStatBlock times(int factor) {
        Map<ElysiumStat, Integer> map = new EnumMap<>(ElysiumStat.class);
        for (ElysiumStat stat : ElysiumStat.values()) {
            int scaled = get(stat) * factor;
            if (scaled != 0) {
                map.put(stat, scaled);
            }
        }
        return new ElysiumStatBlock(map);
    }

    public ElysiumStatBlock with(ElysiumStat stat, int amount) {
        Map<ElysiumStat, Integer> map = new EnumMap<>(ElysiumStat.class);
        map.putAll(values);
        map.put(stat, get(stat) + amount);
        return new ElysiumStatBlock(map);
    }
}
