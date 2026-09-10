package com.elysium.character;

import com.elysium.stats.ElysiumStat;
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
 * What you are.
 *
 * Five of the six come straight out of the Sleeping Empire's Known Species
 * document — the humanoid majority, the reptilian Druun Ascendancy, the avian
 * Veylari Concord, the insectoid Korrath Dominion and the energy-based Lumari
 * Collective — and each one's stat shape is read off what that entry says
 * about them. The Druun are described as militaristic and hierarchical, so
 * they hit hard and think slowly. The Lumari "exist in forms that transcend
 * traditional biology", so they are barely armoured and enormously willed.
 *
 * The sixth, the Unsworn, is the mod's own: the people outside the Code, who
 * already exist here as a faction and a mob.
 *
 * <h2>What a race decides</h2>
 *
 * <ul>
 *   <li><b>A starting block.</b> Every race begins with exactly 44 points,
 *       spread differently. Nobody starts strictly ahead — a property
 *       {@code validate.py} now checks, because the first draft of these
 *       numbers ranged from 41 to 47 and looked perfectly reasonable while it
 *       did.</li>
 *   <li><b>A growth curve.</b> Three points per level, always in the same
 *       stats, so a race's shape sharpens instead of washing out — which is
 *       what happens when levelling gives everyone the same thing.</li>
 *   <li><b>One passive.</b> Something no amount of points can buy.</li>
 * </ul>
 *
 * The growth is what makes the choice permanent enough to matter and the free
 * points (see {@code ElysiumProgression}) are what stop it being a cage.
 */
public enum ElysiumRace {

    /**
     * "The most common form across the Empire. Generally adaptable and
     * widespread." Balanced, and the only race with no weak stat.
     *
     * Their passive is the Empire's own doctrine turned inward: an Imperial
     * standing inside the Code is answered for. Harm done to them is returned
     * automatically, and the share returned grows with level.
     */
    IMPERIAL("imperial", ChatFormatting.GOLD,
            ElysiumStatBlock.of(VITALITY, 4, FORTITUDE, 4, RESILIENCE, 3, STRENGTH, 4,
                    AGILITY, 3, ACCURACY, 3, REFLEXES, 3, RETRIBUTION, 5,
                    INTELLECT, 3, WILLPOWER, 3, LUCK, 3, PRESENCE, 6),
            ElysiumStatBlock.of(RETRIBUTION, 1, PRESENCE, 1, VITALITY, 1),
            Passive.SANCTIONED_ANSWER),

    /**
     * "Including members of the Druun Ascendancy. Known for militaristic
     * tendencies and strong hierarchical societies."
     */
    DRUUN("druun", ChatFormatting.DARK_GREEN,
            ElysiumStatBlock.of(VITALITY, 6, FORTITUDE, 8, RESILIENCE, 5, STRENGTH, 8,
                    AGILITY, 2, ACCURACY, 3, REFLEXES, 2, RETRIBUTION, 2,
                    INTELLECT, 1, WILLPOWER, 3, LUCK, 2, PRESENCE, 2),
            ElysiumStatBlock.of(STRENGTH, 2, FORTITUDE, 1),
            Passive.COLD_BLOOD),

    /**
     * "Including members of the Veylari Concord. Often isolationist and
     * technologically advanced."
     */
    VEYLARI("veylari", ChatFormatting.AQUA,
            ElysiumStatBlock.of(VITALITY, 3, FORTITUDE, 2, RESILIENCE, 2, STRENGTH, 3,
                    AGILITY, 6, ACCURACY, 8, REFLEXES, 5, RETRIBUTION, 1,
                    INTELLECT, 7, WILLPOWER, 2, LUCK, 3, PRESENCE, 2),
            ElysiumStatBlock.of(INTELLECT, 2, ACCURACY, 1),
            Passive.LIGHTFEATHER),

    /**
     * "Including the Korrath Dominion. Known for hive-like social structures
     * and rapid expansion."
     */
    KORRATH("korrath", ChatFormatting.YELLOW,
            ElysiumStatBlock.of(VITALITY, 5, FORTITUDE, 3, RESILIENCE, 3, STRENGTH, 4,
                    AGILITY, 8, ACCURACY, 4, REFLEXES, 7, RETRIBUTION, 2,
                    INTELLECT, 2, WILLPOWER, 1, LUCK, 4, PRESENCE, 1),
            ElysiumStatBlock.of(AGILITY, 2, REFLEXES, 1),
            Passive.MOLT),

    /**
     * "Including the Lumari Collective. Highly advanced and peaceful. Often
     * exist in forms that transcend traditional biology."
     *
     * Which is read here as: almost no body to armour, and an enormous amount
     * of will holding the rest together.
     */
    LUMARI("lumari", ChatFormatting.LIGHT_PURPLE,
            ElysiumStatBlock.of(VITALITY, 2, FORTITUDE, 1, RESILIENCE, 5, STRENGTH, 2,
                    AGILITY, 3, ACCURACY, 3, REFLEXES, 3, RETRIBUTION, 3,
                    INTELLECT, 8, WILLPOWER, 9, LUCK, 3, PRESENCE, 2),
            ElysiumStatBlock.of(WILLPOWER, 2, INTELLECT, 1),
            Passive.PHOTONIC),

    /**
     * Not from the archive — the mod's own. The Unsworn are the people the
     * Code does not cover, and the Empire's regard for them is the whole of
     * their disadvantage: they begin with no Presence at all.
     */
    UNSWORN("unsworn", ChatFormatting.DARK_RED,
            ElysiumStatBlock.of(VITALITY, 4, FORTITUDE, 3, RESILIENCE, 3, STRENGTH, 5,
                    AGILITY, 6, ACCURACY, 4, REFLEXES, 4, RETRIBUTION, 2,
                    INTELLECT, 2, WILLPOWER, 2, LUCK, 9, PRESENCE, 0),
            ElysiumStatBlock.of(LUCK, 2, STRENGTH, 1),
            Passive.UNCOUNTED);

    /** The signature ability. Behaviour lives in {@code ElysiumPassiveHandler}. */
    public enum Passive {
        /** Imperial: reflect a share of every blow, rising with level. */
        SANCTIONED_ANSWER,
        /** Druun: damage climbs as health falls. */
        COLD_BLOOD,
        /** Veylari: fall damage largely ignored; drifts when sneaking in air. */
        LIGHTFEATHER,
        /** Korrath: regeneration surges after a few seconds untouched. */
        MOLT,
        /** Lumari: a standing shield from Willpower; elemental harm blunted. */
        PHOTONIC,
        /** Unsworn: Suspicion sheds twice as fast, Favor comes half as easily. */
        UNCOUNTED
    }

    private final String id;
    private final ChatFormatting colour;
    private final ElysiumStatBlock base;
    private final ElysiumStatBlock growth;
    private final Passive passive;

    ElysiumRace(String id, ChatFormatting colour, ElysiumStatBlock base,
                ElysiumStatBlock growth, Passive passive) {
        this.id = id;
        this.colour = colour;
        this.base = base;
        this.growth = growth;
        this.passive = passive;
    }

    public String getId() {
        return id;
    }

    public ChatFormatting getColour() {
        return colour;
    }

    /** The stats a character of this race starts with at level 1. */
    public ElysiumStatBlock getBaseStats() {
        return base;
    }

    /** What every level adds, before class growth and free points. */
    public ElysiumStatBlock getGrowth() {
        return growth;
    }

    public Passive getPassive() {
        return passive;
    }

    public Component getDisplayName() {
        return Component.translatable("elysium.race." + id).withStyle(colour);
    }

    public Component getDescription() {
        return Component.translatable("elysium.race." + id + ".desc")
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

    /**
     * @return the matching race, or null when the saved name is not one we
     *         recognise — which the caller should read as "unchosen"
     */
    public static ElysiumRace byId(String name) {
        if (name == null || name.isEmpty()) {
            return null;
        }
        for (ElysiumRace candidate : values()) {
            if (candidate.id.equals(name.toLowerCase(Locale.ROOT))) {
                return candidate;
            }
        }
        return null;
    }
}
