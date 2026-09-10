package com.elysium.core.item;

import net.minecraft.core.component.DataComponents;
import net.minecraft.world.item.ItemStack;
import com.elysium.lib.item.ElysiumRarities;

/**
 * Armour ascension: combine two identical pieces to push one of them a tier
 * higher.
 *
 * Rules:
 * <ul>
 *   <li>both pieces must be the same Elysium item;</li>
 *   <li>the piece must not already be at the top tier;</li>
 *   <li>the result keeps the first piece's runes and reforge rolls, and comes
 *       out fully repaired.</li>
 * </ul>
 */
public final class ElysiumArmorAscension {

    private ElysiumArmorAscension() {
    }

    public static boolean canAscend(ItemStack piece1, ItemStack piece2) {
        if (piece1.isEmpty() || piece2.isEmpty()) {
            return false;
        }
        if (piece1.getItem() != piece2.getItem()) {
            return false;
        }
        if (!(piece1.getItem() instanceof ElysiumArmorItem armor)) {
            return false;
        }
        // Both pieces must already be at the same tier. Without this the cost
        // of reaching tier N is N base pieces rather than 2^N, which is the
        // whole of what keeps unbounded ascension from being free — and
        // feeding a tier-0 spare into a Sovereign piece would quietly work.
        if (armor.getEffectiveTier(piece1) != armor.getEffectiveTier(piece2)) {
            return false;
        }
        return armor.canAscend(piece1);
    }

    /**
     * @return the ascended piece, or {@link ItemStack#EMPTY} when the inputs
     *         cannot be ascended
     */
    public static ItemStack ascend(ItemStack piece1, ItemStack piece2) {
        if (!canAscend(piece1, piece2)) {
            return ItemStack.EMPTY;
        }

        ElysiumArmorItem armor = (ElysiumArmorItem) piece1.getItem();
        int nextTier = armor.getNextTier(piece1);

        // copy() carries every component across, so runes and reforge rolls
        // survive the upgrade. The old code rebuilt a bare stack and then
        // re-attached a tag, which silently dropped anything it forgot.
        ItemStack result = piece1.copy();
        result.setCount(1);
        result.setDamageValue(0);

        ElysiumArmorItem.setGearData(result,
                ElysiumArmorItem.gearData(result).withAscendedTier(nextTier));

        // Keep the displayed rarity in step with the new tier.
        result.set(DataComponents.RARITY, ElysiumRarities.getRarityFromTier(nextTier));

        return result;
    }
}
