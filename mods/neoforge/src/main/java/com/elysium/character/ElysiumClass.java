package com.elysium.character;

import com.elysium.stats.ElysiumStatBlock;
import net.minecraft.ChatFormatting;
import net.minecraft.network.chat.Component;

import java.util.Locale;

import static com.elysium.stats.ElysiumStat.ACCURACY;
import static com.elysium.stats.ElysiumStat.AGILITY;
import static com.elysium.stats.ElysiumStat.FORTITUDE;
import static com.elysium.stats.ElysiumStat.INTELLECT;
import static com.elysium.stats.ElysiumStat.LUCK;
import static com.elysium.stats.ElysiumStat.PRESENCE;
import static com.elysium.stats.ElysiumStat.REFLEXES;
import static com.elysium.stats.ElysiumStat.RESILIENCE;
import static com.elysium.stats.ElysiumStat.RETRIBUTION;
import static com.elysium.stats.ElysiumStat.STRENGTH;
import static com.elysium.stats.ElysiumStat.VITALITY;
import static com.elysium.stats.ElysiumStat.WILLPOWER;

/**
 * What you do.
 *
 * Race is biology; class is a job. Both are currently chosen once and kept —
 * a class change belongs at an Ascension Forge with a cost attached, and until
 * that exists there is no free way to swap, because a packet that reassigned a
 * class on demand would let a client wear whichever passive suited the next
 * swing. Six of the nine are lifted from what the
 * archive says the Empire actually employs people to do — Medical
 * Regeneration, Fleet and Infrastructure Engineering, Cybernetic Enhancement,
 * the fleets, the trade apparatus — rather than from a fantasy party sheet.
 *
 * A class contributes two points of growth per level and one passive, against
 * a race's three. The thing you were born as should outweigh the job you took.
 * (The first draft gave three as well, which quietly made the two equal and
 * doubled the documented growth rate — worth re-adding these up after any
 * edit.)
 */
public enum ElysiumClass {

    /**
     * Medical Regeneration, in the field. The archive lists it as an Imperial
     * discipline; here it is the only role that heals anyone other than
     * itself.
     */
    MEDICAE("medicae", ChatFormatting.RED,
            ElysiumStatBlock.of(VITALITY, 1, PRESENCE, 1),
            Passive.TRIAGE_FIELD),

    /**
     * The trade apparatus. A Factor does not fight better — they simply come
     * away with more, and are the only class that improves what other people's
     * kills are worth.
     */
    FACTOR("factor", ChatFormatting.GOLD,
            ElysiumStatBlock.of(LUCK, 1, PRESENCE, 1),
            Passive.PROFITEER),

    /**
     * Fleet and Infrastructure Engineering. Keeps gear alive and gets more out
     * of a reforge than anyone else does.
     */
    ARTIFICER("artificer", ChatFormatting.AQUA,
            ElysiumStatBlock.of(INTELLECT, 1, PRESENCE, 1),
            Passive.FIELD_REPAIR),

    /** The fleets' line soldier, and the Code's blunt instrument. */
    ENFORCER("enforcer", ChatFormatting.DARK_RED,
            ElysiumStatBlock.of(STRENGTH, 1, FORTITUDE, 1),
            Passive.SANCTIONED_FORCE),

    /** The psionic elemental system, practised rather than merely carried. */
    PSION("psion", ChatFormatting.DARK_PURPLE,
            ElysiumStatBlock.of(INTELLECT, 1, WILLPOWER, 1),
            Passive.RESONANCE),

    /** Cybernetic Enhancement, spent entirely on getting somewhere first. */
    VOIDRUNNER("voidrunner", ChatFormatting.GREEN,
            ElysiumStatBlock.of(AGILITY, 1, REFLEXES, 1),
            Passive.SLIPSTREAM),

    /**
     * Singularity-Forged Neutronium has to come from somewhere. Reclaimers are
     * the ones underground getting it.
     */
    RECLAIMER("reclaimer", ChatFormatting.YELLOW,
            ElysiumStatBlock.of(FORTITUDE, 1, LUCK, 1),
            Passive.PROSPECTOR),

    /**
     * Modelled on Sentinel: the thing that stands in the way when nobody is
     * left to give orders.
     */
    WARDEN("warden", ChatFormatting.BLUE,
            ElysiumStatBlock.of(RESILIENCE, 1, RETRIBUTION, 1),
            Passive.BULWARK),

    /** Accuracy over everything. The long shot, taken from cover. */
    MARKSMAN("marksman", ChatFormatting.WHITE,
            ElysiumStatBlock.of(ACCURACY, 1, AGILITY, 1),
            Passive.CALLED_SHOT);

    /** Behaviour lives in {@code ElysiumPassiveHandler}. */
    public enum Passive {
        /** Medicae: heals nearby players, and itself faster. */
        TRIAGE_FIELD,
        /** Factor: Elysium drops sometimes come doubled. */
        PROFITEER,
        /** Artificer: Elysium gear wears slower and reforges better. */
        FIELD_REPAIR,
        /** Enforcer: more damage to the Unsworn, less Suspicion earned. */
        SANCTIONED_FORCE,
        /** Psion: aligned runes count twice, elemental advantage widened. */
        RESONANCE,
        /** Voidrunner: faster while sprinting, and lands softly. */
        SLIPSTREAM,
        /** Reclaimer: Elysium ore breaks faster and sometimes pays double. */
        PROSPECTOR,
        /** Warden: retribution doubles below half health. */
        BULWARK,
        /** Marksman: a critical hit hits considerably harder. */
        CALLED_SHOT
    }

    private final String id;
    private final ChatFormatting colour;
    private final ElysiumStatBlock growth;
    private final Passive passive;

    ElysiumClass(String id, ChatFormatting colour, ElysiumStatBlock growth, Passive passive) {
        this.id = id;
        this.colour = colour;
        this.growth = growth;
        this.passive = passive;
    }

    public String getId() {
        return id;
    }

    public ChatFormatting getColour() {
        return colour;
    }

    /** What every level adds on top of the race's own growth. */
    public ElysiumStatBlock getGrowth() {
        return growth;
    }

    public Passive getPassive() {
        return passive;
    }

    public Component getDisplayName() {
        return Component.translatable("elysium.class." + id).withStyle(colour);
    }

    public Component getDescription() {
        return Component.translatable("elysium.class." + id + ".desc")
                .withStyle(ChatFormatting.GRAY);
    }

    public Component getPassiveName() {
        return Component.translatable("elysium.passive." + passive.name().toLowerCase(Locale.ROOT))
                .withStyle(colour);
    }

    public Component getPassiveDescription() {
        return Component.translatable(
                        "elysium.passive." + passive.name().toLowerCase(Locale.ROOT) + ".desc")
                .withStyle(ChatFormatting.DARK_GRAY);
    }

    public static ElysiumClass byId(String name) {
        if (name == null || name.isEmpty()) {
            return null;
        }
        for (ElysiumClass candidate : values()) {
            if (candidate.id.equals(name.toLowerCase(Locale.ROOT))) {
                return candidate;
            }
        }
        return null;
    }
}
