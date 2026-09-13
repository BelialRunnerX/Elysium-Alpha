package com.elysium.network;

import com.elysium.character.ElysiumCharacter;
import com.elysium.character.ElysiumClass;
import com.elysium.character.ElysiumRace;
import com.elysium.stats.ElysiumStat;
import com.elysium.stats.ElysiumStatBlock;
import com.elysium.stats.ElysiumStats;
import net.minecraft.world.entity.player.Player;

import java.util.EnumMap;
import java.util.Map;

/**
 * A character, flattened to one string and back.
 *
 * <pre>{@code   race|class|level|xp|xpNext|stat:value,stat:value,...   }</pre>
 *
 * Deliberately boring. The alternative — a fifteen-field packet — has to agree
 * on field order between two sides that ship independently, and gets a field
 * added every time a stat does. This format survives a stat being added (the
 * old client ignores what it does not know) and a stat being removed (the
 * value parses to a name nothing matches, and is dropped).
 *
 * It is a display snapshot and nothing more. Every value here is recomputed
 * server-side before it is ever acted on; nothing a client sends back is
 * trusted.
 */
public final class CharacterSheet {

    private CharacterSheet() {
    }

    private static final String FIELD = "\\|";
    private static final String PAIR = ",";

    public static String pack(Player player) {
        ElysiumRace race = ElysiumCharacter.getRace(player);
        ElysiumClass job = ElysiumCharacter.getElysiumClass(player);
        int level = ElysiumCharacter.getLevel(player);

        StringBuilder stats = new StringBuilder();
        ElysiumStatBlock total = ElysiumStats.total(player);
        for (ElysiumStat stat : ElysiumStat.values()) {
            if (stats.length() > 0) {
                stats.append(PAIR);
            }
            stats.append(stat.getId()).append(':').append(total.get(stat));
        }

        return String.join("|",
                race == null ? "" : race.getId(),
                job == null ? "" : job.getId(),
                Integer.toString(level),
                Integer.toString(ElysiumCharacter.getXp(player)),
                Integer.toString(ElysiumCharacter.xpToNext(level)),
                stats.toString());
    }

    /** The client-side view of a packed sheet. */
    public record Parsed(ElysiumRace race,
                         ElysiumClass job,
                         int level,
                         int xp,
                         int xpNext,
                         Map<ElysiumStat, Integer> stats) {

        public int get(ElysiumStat stat) {
            Integer value = stats.get(stat);
            return value == null ? 0 : value;
        }

        public boolean chosen() {
            return race != null && job != null;
        }
    }

    /**
     * Parses a packed sheet.
     *
     * Tolerant on purpose: a short or malformed string yields an empty sheet
     * rather than an exception, because the one thing worse than a wrong
     * character screen is a client that disconnects trying to draw one.
     */
    public static Parsed parse(String packed) {
        Map<ElysiumStat, Integer> stats = new EnumMap<>(ElysiumStat.class);
        if (packed == null || packed.isEmpty()) {
            return new Parsed(null, null, 1, 0, 1, stats);
        }

        String[] fields = packed.split(FIELD, -1);
        ElysiumRace race = fields.length > 0 ? ElysiumRace.byId(fields[0]) : null;
        ElysiumClass job = fields.length > 1 ? ElysiumClass.byId(fields[1]) : null;
        int level = fields.length > 2 ? parseInt(fields[2], 1) : 1;
        int xp = fields.length > 3 ? parseInt(fields[3], 0) : 0;
        int xpNext = fields.length > 4 ? parseInt(fields[4], 1) : 1;

        if (fields.length > 5 && !fields[5].isEmpty()) {
            for (String entry : fields[5].split(PAIR)) {
                int colon = entry.indexOf(':');
                if (colon <= 0) {
                    continue;
                }
                ElysiumStat stat = ElysiumStat.byId(entry.substring(0, colon));
                if (stat != null) {
                    stats.put(stat, parseInt(entry.substring(colon + 1), 0));
                }
            }
        }

        return new Parsed(race, job, level, xp, Math.max(1, xpNext), stats);
    }

    private static int parseInt(String text, int fallback) {
        try {
            return Integer.parseInt(text.trim());
        } catch (NumberFormatException ignored) {
            return fallback;
        }
    }
}
