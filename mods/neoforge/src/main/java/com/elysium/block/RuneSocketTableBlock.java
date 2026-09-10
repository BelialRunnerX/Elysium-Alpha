package com.elysium.block;

import net.minecraft.world.level.block.Block;
import net.minecraft.world.level.block.SoundType;
import net.minecraft.world.level.block.state.BlockBehaviour;
import net.minecraft.world.level.material.MapColor;

public class RuneSocketTableBlock extends Block {
    public RuneSocketTableBlock() {
        super(BlockBehaviour.Properties.of()
            .mapColor(MapColor.METAL)
            .strength(5.0F, 1200.0F)
            .requiresCorrectToolForDrops()
            .sound(SoundType.STONE));
    }
}