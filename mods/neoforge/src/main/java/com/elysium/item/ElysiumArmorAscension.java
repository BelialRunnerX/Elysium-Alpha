package com.elysium.item;

import net.minecraft.world.item.ItemStack;
import net.minecraft.world.item.Rarity;

/**
 * Handles armor ascension logic.
 * 
 * This class manages the process of upgrading an armor piece to a higher rarity tier
 * by combining two identical pieces.
 * 
 * Ascension Rules:
 * - Two identical armor pieces are required
 * - The resulting piece will be one tier higher
 * - Unique (tier 5) items cannot be ascended
 */
public class ElysiumArmorAscension {

    /**
     * Checks if two armor pieces can be ascended.
     * 
     * @param piece1 First armor piece
     * @param piece2 Second armor piece
     * @return true if ascension is possible
     */
    public static boolean canAscend(ItemStack piece1, ItemStack piece2) {
        if (piece1.getItem() != piece2.getItem()) return false;
        if (!(piece1.getItem() instanceof ElysiumArmorItem)) return false;

        ElysiumArmorItem armor = (ElysiumArmorItem) piece1.getItem();
        return armor.canAscend();
    }

    /**
     * Performs ascension and returns the upgraded armor.
     * 
     * @param piece1 First armor piece
     * @param piece2 Second armor piece
     * @return Upgraded armor, or empty stack if ascension fails
     */
    public static ItemStack ascend(ItemStack piece1, ItemStack piece2) {
        if (!canAscend(piece1, piece2)) return ItemStack.EMPTY;

        ElysiumArmorItem armor = (ElysiumArmorItem) piece1.getItem();
        int nextTier = armor.getNextTier();

        // Create new armor with upgraded tier (same element)
        ItemStack result = new ItemStack(armor, 1);
        
        // Copy NBT data from the first piece (runes, reforged stats, etc.)
        if (piece1.hasTag()) {
            result.setTag(piece1.getTag().copy());
        }
        
        // Update the tier in NBT (we'll use a custom tag for this)
        result.getOrCreateTag().putInt("ElysiumAscendedTier", nextTier);
        
        return result;
    }
}