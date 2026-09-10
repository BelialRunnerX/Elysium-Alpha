package com.elysium.item;

import com.elysium.Elysium;
import net.minecraft.core.component.DataComponentType;
import net.neoforged.neoforge.registries.DeferredHolder;
import net.neoforged.neoforge.registries.DeferredRegister;

/**
 * Custom data components. This is the 1.20.5+ replacement for the loose NBT
 * tags the mod used to write directly onto item stacks.
 */
public final class ElysiumComponents {

    private ElysiumComponents() {
    }

    public static final DeferredRegister.DataComponents COMPONENTS =
            DeferredRegister.createDataComponents(Elysium.MODID);

    public static final DeferredHolder<DataComponentType<?>, DataComponentType<ElysiumGearData>> GEAR_DATA =
            COMPONENTS.registerComponentType("gear_data", builder -> builder
                    .persistent(ElysiumGearData.CODEC)
                    .networkSynchronized(ElysiumGearData.STREAM_CODEC));
}
