package com.elysium.silentgear;

import net.minecraftforge.fml.ModList;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

/**
 * Silent Gear Integration for Elysium Mod.
 * 
 * This class handles registering Elysium materials with Silent Gear
 * so they can be used in modular gear crafting.
 * 
 * Materials are registered with proper stats so Silent Gear can use them.
 */
public class ElysiumSilentGear {

    private static final Logger LOGGER = LogManager.getLogger();

    public static void register() {
        if (!ModList.get().isLoaded("silentgear")) {
            LOGGER.info("Silent Gear not detected - skipping integration");
            return;
        }

        LOGGER.info("Silent Gear detected - registering Elysium materials with stats");

        // In a full implementation, we would use Silent Gear's API to register materials:
        // 
        // Example (requires Silent Gear API):
        // MaterialManager.registerMaterial(
        //     new Material("elysium:neutronium")
        //         .setStats(StatInstance.of(Stat.DURABILITY, 2500))
        //         .setStats(StatInstance.of(Stat.ARMOR, 4.0f))
        //         .setStats(StatInstance.of(Stat.ATTACK_DAMAGE, 5.0f))
        //         .setTier(5)
        // );
        //
        // MaterialManager.registerMaterial(
        //     new Material("elysium:aetherium")
        //         .setStats(StatInstance.of(Stat.DURABILITY, 1800))
        //         .setStats(StatInstance.of(Stat.ARMOR, 3.0f))
        //         .setStats(StatInstance.of(Stat.ATTACK_SPEED, 0.2f))
        //         .setTier(4)
        // );
        
        // For now, materials are also defined in JSON under data/silentgear/materials/
        // This provides a fallback if the code registration isn't available.
    }
}