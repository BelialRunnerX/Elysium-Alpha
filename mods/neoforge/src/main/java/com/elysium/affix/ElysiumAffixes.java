package com.elysium.affix;

import net.minecraftforge.eventbus.api.IEventBus;
import shadows.apotheosis.adventure.affix.AffixRegistry;
import net.minecraft.resources.ResourceLocation;

public class ElysiumAffixes {

    public static void register(IEventBus bus) {
        // Register Elysium psionic-themed affixes with Apotheosis
        // These affixes will only apply to Elysium armor and provide elemental bonuses

        // Void Resistance Affix
        AffixRegistry.INSTANCE.register(
            new ResourceLocation("elysium", "void_resistance"),
            new ElysiumPsionicAffix("Void", 0.10f, 0.25f)
        );

        // Plasma Damage Affix
        AffixRegistry.INSTANCE.register(
            new ResourceLocation("elysium", "plasma_damage"),
            new ElysiumPsionicAffix("Plasma", 0.08f, 0.20f)
        );

        // Neural Haste Affix
        AffixRegistry.INSTANCE.register(
            new ResourceLocation("elysium", "neural_haste"),
            new ElysiumPsionicAffix("Neural", 0.05f, 0.15f)
        );

        // Dimensional Mobility Affix
        AffixRegistry.INSTANCE.register(
            new ResourceLocation("elysium", "dimensional_mobility"),
            new ElysiumPsionicAffix("Dimensional", 0.06f, 0.18f)
        );

        // Kinetic Force Affix
        AffixRegistry.INSTANCE.register(
            new ResourceLocation("elysium", "kinetic_force"),
            new ElysiumPsionicAffix("Kinetic", 0.07f, 0.22f)
        );
    }
}