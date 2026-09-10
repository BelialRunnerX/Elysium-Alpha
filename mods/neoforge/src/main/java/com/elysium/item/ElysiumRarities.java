package com.elysium.item;

import net.minecraft.world.item.Rarity;
import net.minecraft.ChatFormatting;

public class ElysiumRarities {

    public static final Rarity COMMON = Rarity.create("elysium_common", ChatFormatting.WHITE);
    public static final Rarity UNCOMMON = Rarity.create("elysium_uncommon", ChatFormatting.GREEN);
    public static final Rarity RARE = Rarity.create("elysium_rare", ChatFormatting.BLUE);
    public static final Rarity EPIC = Rarity.create("elysium_epic", ChatFormatting.LIGHT_PURPLE);
    public static final Rarity LEGENDARY = Rarity.create("elysium_legendary", ChatFormatting.GOLD);
    public static final Rarity UNIQUE = Rarity.create("elysium_unique", ChatFormatting.GOLD);

    public static Rarity getRarityFromTier(int tier) {
        return switch (tier) {
            case 0 -> COMMON;
            case 1 -> UNCOMMON;
            case 2 -> RARE;
            case 3 -> EPIC;
            case 4 -> LEGENDARY;
            case 5 -> UNIQUE;
            default -> COMMON;
        };
    }
}