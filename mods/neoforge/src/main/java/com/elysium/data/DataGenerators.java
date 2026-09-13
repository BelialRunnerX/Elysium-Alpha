package com.elysium.data;

import com.elysium.Elysium;
import net.minecraft.data.DataGenerator;
import net.minecraftforge.data.event.GatherDataEvent;
import net.minecraftforge.eventbus.api.SubscribeEvent;
import net.minecraftforge.fml.common.Mod;

@Mod.EventBusSubscriber(modid = Elysium.MODID, bus = Mod.EventBusSubscriber.Bus.MOD)
public class DataGenerators {

    @SubscribeEvent
    public static void gatherData(GatherDataEvent event) {
        DataGenerator generator = event.getGenerator();
        var existingFileHelper = event.getExistingFileHelper();

        // Register providers
        generator.addProvider(event.includeServer(), new ElysiumRecipeProvider(generator));
        generator.addProvider(event.includeClient(), new ElysiumBlockStateProvider(generator, existingFileHelper));
    }
}