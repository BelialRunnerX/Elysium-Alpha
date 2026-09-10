package com.elysium.item;

import com.elysium.element.ElysiumElement;
import net.minecraft.world.item.Tier;
import net.minecraft.world.item.Tiers;

/**
 * The three materials Elysium tools and weapons are forged from, and the
 * psionic element each one resonates with.
 *
 * That pairing is what makes rune alignment mean something. A rune socketed
 * into gear of its own element bites harder than the same rune in gear that
 * merely tolerates it — see {@link ElysiumSockets#ALIGNED_MULTIPLIER}.
 *
 * Plasma and Neural deliberately have no material of their own. Their gear
 * comes from the elemental weapon line and the armour set instead, so no single
 * material lets a player align every rune they own.
 */
public enum ElysiumMaterial {

    /** Cut, not cast. Resonates with Void. */
    VOIDGLASS("voidglass", ElysiumElement.VOID, ElysiumRarities.RARE, Tiers.DIAMOND, 0.0F),

    /** Light and planar. Resonates with Dimensional. */
    AETHERIUM("aetherium", ElysiumElement.DIMENSIONAL, ElysiumRarities.EPIC, Tiers.DIAMOND, 1.0F),

    /** Absurdly dense. Resonates with Kinetic. */
    NEUTRONIUM("neutronium", ElysiumElement.KINETIC, ElysiumRarities.LEGENDARY, Tiers.NETHERITE, 2.0F);

    private final String id;
    private final ElysiumElement element;
    private final int tier;
    private final Tier toolTier;
    private final float damageBonus;

    ElysiumMaterial(String id, ElysiumElement element, int tier, Tier toolTier, float damageBonus) {
        this.id = id;
        this.element = element;
        this.tier = tier;
        this.toolTier = toolTier;
        this.damageBonus = damageBonus;
    }

    public String getId() {
        return id;
    }

    public ElysiumElement getElement() {
        return element;
    }

    /** The Elysium gear tier, which sets rarity and elemental advantage size. */
    public int getTier() {
        return tier;
    }

    /** The vanilla tier, which sets mining level, speed and durability. */
    public Tier getToolTier() {
        return toolTier;
    }

    /** Added on top of each tool shape's own attack damage. */
    public float getDamageBonus() {
        return damageBonus;
    }
}
