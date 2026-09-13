package com.elysium.block;

import net.minecraft.world.level.block.Block;
import net.minecraft.world.level.block.SoundType;
import net.minecraft.world.level.block.state.BlockBehaviour;
import net.minecraft.world.level.material.MapColor;

public class AscensionForgeBlock extends Block {
    public AscensionForgeBlock() {
        super(BlockBehaviour.Properties.of()
            .mapColor(MapColor.METAL)
            .strength(6.0F, 1200.0F)
            .requiresCorrectToolForDrops()
            .sound(SoundType.ANVIL));
    }
}