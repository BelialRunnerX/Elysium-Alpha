package com.elysium.data;

import com.elysium.Elysium;
import net.minecraft.data.DataGenerator;
import net.minecraftforge.client.model.generators.BlockStateProvider;
import net.minecraftforge.common.data.ExistingFileHelper;

public class ElysiumBlockStateProvider extends BlockStateProvider {

    public ElysiumBlockStateProvider(DataGenerator generator, ExistingFileHelper existingFileHelper) {
        super(generator, Elysium.MODID, existingFileHelper);
    }

    @Override
    protected void registerStatesAndModels() {
        // Neutronium
        simpleBlock(Elysium.NEUTRONIUM_BLOCK.get());
        simpleBlock(Elysium.NEUTRONIUM_ORE.get());
        
        // Register item models
        itemModels().withExistingParent(Elysium.NEUTRONIUM_BLOCK_ITEM.getId().getPath(), 
            modLoc("block/neutronium_block"));
        itemModels().withExistingParent(Elysium.NEUTRONIUM_ORE_ITEM.getId().getPath(), 
            modLoc("block/neutronium_ore"));
    }
}