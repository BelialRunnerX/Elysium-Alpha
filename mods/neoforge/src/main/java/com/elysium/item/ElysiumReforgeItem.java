package com.elysium.item;

import net.minecraft.world.item.Item;
import net.minecraft.world.item.ItemStack;
import net.minecraft.world.item.Rarity;

public class ElysiumReforgeItem extends Item {

    public ElysiumReforgeItem(Properties properties) {
        super(properties);
    }

    @Override
    public Rarity getRarity(ItemStack stack) {
        return Rarity.EPIC;
    }

    /**
     * Determines the quality multiplier based on Silent Gear material grade.
     * Higher grades give better stat rolls during reforging.
     */
    public static float getGradeMultiplier(String materialGrade) {
        return switch (materialGrade.toLowerCase()) {
            case "crude" -> 0.8f;
            case "common" -> 1.0f;
            case "uncommon" -> 1.2f;
            case "rare" -> 1.5f;
            case "epic" -> 1.8f;
            case "legendary" -> 2.2f;
            default -> 1.0f;
        };
    }
}