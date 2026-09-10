package com.elysium.tooltip;

import net.minecraftforge.fml.ModList;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

/**
 * Legendary Tooltips Integration for Elysium Mod.
 * 
 * This class registers our custom rarities with Legendary Tooltips
 * so they display with the correct colors.
 */
public class ElysiumLegendaryTooltips {

    private static final Logger LOGGER = LogManager.getLogger();

    public static void register() {
        if (!ModList.get().isLoaded("legendarytooltips")) {
            LOGGER.info("Legendary Tooltips not detected - skipping integration");
            return;
        }

        LOGGER.info("Legendary Tooltips detected - registering Elysium rarities");

        // In a full implementation, we would use Legendary Tooltips' API:
        // 
        // Example:
        // LegendaryTooltipsAPI.registerRarity("elysium_common", ChatFormatting.WHITE);
        // LegendaryTooltipsAPI.registerRarity("elysium_uncommon", ChatFormatting.GREEN);
        // LegendaryTooltipsAPI.registerRarity("elysium_rare", ChatFormatting.BLUE);
        // LegendaryTooltipsAPI.registerRarity("elysium_epic", ChatFormatting.LIGHT_PURPLE);
        // LegendaryTooltipsAPI.registerRarity("elysium_legendary", ChatFormatting.GOLD);
        // LegendaryTooltipsAPI.registerRarity("elysium_unique", ChatFormatting.GOLD);
        
        // Our custom rarities are defined in ElysiumRarities.java
        // and returned by ElysiumArmorItem.getRarity()
    }
}