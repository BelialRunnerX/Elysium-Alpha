package com.elysium.item;

import net.minecraft.world.item.ItemStack;
import java.util.Random;

/**
 * Handles the reforge mechanic for Elysium armor.
 * 
 * This class is responsible for rerolling the base stats of Elysium armor
 * when a player uses a reforge item or material.
 * 
 * Key behaviors:
 * - Base stats are rerolled using materials of the same type as the armor.
 * - Higher Silent Gear grades give better stat rolls.
 * - Armor rarity influences the quality of the rolls.
 */
public class ElysiumReforgeHandler {

    private static final Random RANDOM = new Random();

    /**
     * Reforges an armor piece using a material.
     * 
     * @param armor The armor item to be reforged
     * @param material The material used for reforging (can be a Silent Gear material)
     * @return A new ItemStack with reforged stats, or the original if reforging fails
     */
    public static ItemStack reforge(ItemStack armor, ItemStack material) {
        if (!(armor.getItem() instanceof ElysiumArmorItem)) {
            return ItemStack.EMPTY;
        }

        ElysiumArmorItem armorItem = (ElysiumArmorItem) armor.getItem();
        int tier = armorItem.getTier();
        int rarityMultiplier = getRarityMultiplier(armorItem.getRarity(armor).ordinal());

        // Get grade multiplier from Silent Gear material (if applicable)
        float gradeMultiplier = 1.0f;
        // TODO: Detect Silent Gear material and get its grade for better rolls

        int basePoints = getBaseStatPoints(tier);
        int finalPoints = (int) (basePoints * rarityMultiplier * gradeMultiplier);

        // Create a copy of the armor to apply changes to
        ItemStack result = armor.copy();
        
        // Distribute points randomly across stats
        int armorBonus = RANDOM.nextInt(finalPoints / 2) + 1;
        int healthBonus = RANDOM.nextInt(finalPoints / 2) + 1;
        int speedBonus = RANDOM.nextInt(finalPoints / 3) + 1;
        
        net.minecraft.nbt.CompoundTag tag = result.getOrCreateTag();
        tag.putInt("ElysiumReforgedArmor", armorBonus);
        tag.putInt("ElysiumReforgedHealth", healthBonus);
        tag.putInt("ElysiumReforgedSpeed", speedBonus);
        tag.putBoolean("ElysiumReforged", true);
        
        return result;
    }

    private static int getBaseStatPoints(int tier) {
        return switch (tier) {
            case 0 -> 3;   // Common
            case 1 -> 5;   // Uncommon
            case 2 -> 8;   // Rare
            case 3 -> 12;  // Epic
            case 4 -> 18;  // Legendary
            case 5 -> 25;  // Unique
            default -> 3;
        };
    }

    private static int getRarityMultiplier(int rarityOrdinal) {
        // Rarity ordinal: 0=Common, 1=Uncommon, 2=Rare, 3=Epic, 4=Legendary, 5=Unique
        return switch (rarityOrdinal) {
            case 0 -> 1;
            case 1 -> 2;
            case 2 -> 3;
            case 3 -> 4;
            case 4 -> 5;
            case 5 -> 6;
            default -> 1;
        };
    }
}