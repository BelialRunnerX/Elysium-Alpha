package com.elysium.element;

import net.minecraft.ChatFormatting;
import net.minecraft.network.chat.Component;

/**
 * The five psionic elements, and how they answer one another.
 *
 * The counter matrix is read straight out of the Sleeping Empire equipment
 * archive. Two entries there pin it down:
 *
 * <pre>
 *   Singularity Lance    (Dimensional)  +30% against Plasma and Neural armor
 *   Neural Cascade Rifle (Neural)       +20% against Void armor
 * </pre>
 *
 * Both fall out of a single rule: arrange the elements in the cycle
 *
 * <pre>
 *   Void → Plasma → Neural → Dimensional → Kinetic → Void
 * </pre>
 *
 * and each element counters <em>the two that precede it</em>. Dimensional
 * therefore answers Neural and Plasma; Neural answers Plasma and Void. Every
 * element counters two and is countered by two, so no element is dead weight.
 *
 * The archive's two figures differ (30% vs 20%) even though both are primary
 * counters, and the reason is visible in the entries themselves: the Lance is
 * Legendary and the Rifle is Epic. So the matrix decides <em>whether</em> you
 * have the advantage, and the item's tier decides <em>how much</em>.
 */
public enum ElysiumElement {

    VOID("void", ChatFormatting.LIGHT_PURPLE),
    PLASMA("plasma", ChatFormatting.GOLD),
    NEURAL("neural", ChatFormatting.GREEN),
    DIMENSIONAL("dimensional", ChatFormatting.AQUA),
    KINETIC("kinetic", ChatFormatting.YELLOW),
    /** Neutronium and vanilla gear: no affinity, no advantage either way. */
    NONE("none", ChatFormatting.GRAY);

    /** The elements that take part in the cycle, in cycle order. */
    private static final ElysiumElement[] CYCLE = {VOID, PLASMA, NEURAL, DIMENSIONAL, KINETIC};

    private final String id;
    private final ChatFormatting colour;

    ElysiumElement(String id, ChatFormatting colour) {
        this.id = id;
        this.colour = colour;
    }

    public String getId() {
        return id;
    }

    public ChatFormatting getColour() {
        return colour;
    }

    public boolean isElemental() {
        return this != NONE;
    }

    public Component getDisplayName() {
        return Component.translatable("elysium.element." + id).withStyle(colour);
    }

    /**
     * @return true when this element holds the advantage over {@code other}
     */
    public boolean isStrongAgainst(ElysiumElement other) {
        if (!this.isElemental() || other == null || !other.isElemental()) {
            return false;
        }
        int distance = Math.floorMod(this.ordinal() - other.ordinal(), CYCLE.length);
        return distance == 1 || distance == 2;
    }

    /** The two elements this one counters, for tooltips. */
    public ElysiumElement[] counters() {
        if (!isElemental()) {
            return new ElysiumElement[0];
        }
        return new ElysiumElement[]{
                CYCLE[Math.floorMod(this.ordinal() - 1, CYCLE.length)],
                CYCLE[Math.floorMod(this.ordinal() - 2, CYCLE.length)],
        };
    }

    public static ElysiumElement byId(String id) {
        if (id != null) {
            for (ElysiumElement element : values()) {
                if (element.id.equals(id)) {
                    return element;
                }
            }
        }
        return NONE;
    }

    /**
     * Legacy lookup. Elements used to be raw ints scattered through the item
     * registrations; this keeps any stored data readable.
     */
    public static ElysiumElement byOrdinal(int ordinal) {
        return ordinal >= 0 && ordinal < CYCLE.length ? CYCLE[ordinal] : NONE;
    }
}
