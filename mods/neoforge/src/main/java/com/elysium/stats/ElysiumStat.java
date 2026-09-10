package com.elysium.stats;

import net.minecraft.ChatFormatting;
import net.minecraft.network.chat.Component;

import java.util.Locale;

/**
 * The twelve numbers a character is made of.
 *
 * <h2>Why nothing here has a cap</h2>
 *
 * Reforging and ascension are meant to climb forever, which means stats climb
 * forever, which means a percentage stat has to be able to take any input
 * without ever reaching 100%. Every proportional stat therefore reads through
 * the curve
 *
 * <pre>{@code   value / (value + K)   }</pre>
 *
 * which rises steeply at first, slows down, and approaches 1.0 without ever
 * arriving. Doubling Resilience always helps and never finishes the job. A
 * player at 10,000 Resilience is close to immune and still not immune, and the
 * arithmetic never overflows into nonsense.
 *
 * Flat stats — Strength, Fortitude, Vitality — grow linearly and are allowed
 * to, because the things they feed (damage, armour, health) are already
 * balanced against enemies that also scale with the player's standing.
 *
 * <h2>Where the names come from</h2>
 *
 * The Sleeping Empire's character sheet keeps five: Strength, Reflexes,
 * Intelligence, Willpower and Presence. All five are here — Intelligence as
 * Intellect — alongside the seven the mod needs to express its own systems.
 */
public enum ElysiumStat {

    /** Passive health regeneration, and a little maximum health with it. */
    VITALITY("vitality", Shape.FLAT, ChatFormatting.RED),

    /** Base armour with nothing equipped. Being tough before being armoured. */
    FORTITUDE("fortitude", Shape.FLAT, ChatFormatting.GRAY),

    /** Flat proportional damage reduction, applied after armour. */
    RESILIENCE("resilience", Shape.CURVE, ChatFormatting.DARK_AQUA),

    /** Base attack damage. Weapons multiply this — see {@link ElysiumStats#baseDamage}. */
    STRENGTH("strength", Shape.FLAT, ChatFormatting.DARK_RED),

    /** Movement speed. */
    AGILITY("agility", Shape.CURVE, ChatFormatting.GREEN),

    /** Critical hit chance. */
    ACCURACY("accuracy", Shape.CURVE, ChatFormatting.YELLOW),

    /** Chance to avoid a blow outright. The archive calls this Reflexes. */
    REFLEXES("reflexes", Shape.CURVE, ChatFormatting.AQUA),

    /** The share of incoming damage sent back to whoever dealt it. */
    RETRIBUTION("retribution", Shape.CURVE, ChatFormatting.LIGHT_PURPLE),

    /** Psionic potency: elemental advantage size and rune strength. */
    INTELLECT("intellect", Shape.FLAT, ChatFormatting.BLUE),

    /** Shield capacity and how fast it rebuilds. */
    WILLPOWER("willpower", Shape.FLAT, ChatFormatting.DARK_PURPLE),

    /** Loot chance modifier. */
    LUCK("luck", Shape.CURVE, ChatFormatting.GOLD),

    /** How fast standing moves, and how well a reforge rolls. */
    PRESENCE("presence", Shape.FLAT, ChatFormatting.WHITE);

    /** How a stat's raw points turn into an effect. */
    public enum Shape {
        /** Grows without limit, linearly. */
        FLAT,
        /** Reads through {@code v/(v+K)} — always rising, never arriving. */
        CURVE
    }

    private final String id;
    private final Shape shape;
    private final ChatFormatting colour;

    ElysiumStat(String id, Shape shape, ChatFormatting colour) {
        this.id = id;
        this.shape = shape;
        this.colour = colour;
    }

    public String getId() {
        return id;
    }

    public Shape getShape() {
        return shape;
    }

    public ChatFormatting getColour() {
        return colour;
    }

    public Component getDisplayName() {
        return Component.translatable("elysium.stat." + id).withStyle(colour);
    }

    /** The tooltip line describing what this stat does. */
    public Component getDescription() {
        return Component.translatable("elysium.stat." + id + ".desc")
                .withStyle(ChatFormatting.DARK_GRAY);
    }

    /**
     * @return the matching stat, or null for a name we no longer recognise —
     *         an older save, or a stat removed by an add-on
     */
    public static ElysiumStat byId(String name) {
        if (name == null) {
            return null;
        }
        for (ElysiumStat candidate : values()) {
            if (candidate.id.equals(name.toLowerCase(Locale.ROOT))) {
                return candidate;
            }
        }
        return null;
    }
}
