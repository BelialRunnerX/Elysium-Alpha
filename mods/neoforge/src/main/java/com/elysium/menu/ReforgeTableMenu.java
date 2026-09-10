package com.elysium.menu;

import com.elysium.Elysium;
import com.elysium.item.ElysiumArmorItem;
import com.elysium.item.ElysiumReforgeHandler;
import com.elysium.item.ElysiumRuneItem;
import net.minecraft.network.FriendlyByteBuf;
import net.minecraft.world.entity.player.Inventory;
import net.minecraft.world.entity.player.Player;
import net.minecraft.world.inventory.AbstractContainerMenu;
import net.minecraft.world.inventory.Slot;
import net.minecraft.world.item.ItemStack;
import net.minecraft.world.level.block.entity.BlockEntity;
import net.minecraftforge.items.IItemHandler;
import net.minecraftforge.items.ItemStackHandler;
import net.minecraftforge.items.SlotItemHandler;

public class ReforgeTableMenu extends AbstractContainerMenu {

    private final BlockEntity blockEntity;
    private final ItemStackHandler itemHandler = new ItemStackHandler(3); // Armor, Material, Rune

    public ReforgeTableMenu(int containerId, Inventory playerInventory, BlockEntity blockEntity) {
        super(Elysium.REFORGE_TABLE_MENU.get(), containerId);
        this.blockEntity = blockEntity;

        // Armor slot
        this.addSlot(new SlotItemHandler(itemHandler, 0, 30, 35) {
            @Override
            public boolean mayPlace(ItemStack stack) {
                return stack.getItem() instanceof ElysiumArmorItem;
            }
        });

        // Material slot (for reforging)
        this.addSlot(new SlotItemHandler(itemHandler, 1, 80, 35));

        // Rune slot (for socketing)
        this.addSlot(new SlotItemHandler(itemHandler, 2, 130, 35) {
            @Override
            public boolean mayPlace(ItemStack stack) {
                return stack.getItem() instanceof ElysiumRuneItem;
            }
        });

        // Player inventory
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 9; ++j) {
                this.addSlot(new Slot(playerInventory, j + i * 9 + 9, 8 + j * 18, 84 + i * 18));
            }
        }

        for (int k = 0; k < 9; ++k) {
            this.addSlot(new Slot(playerInventory, k, 8 + k * 18, 142));
        }
    }

    public ReforgeTableMenu(int containerId, Inventory playerInventory, FriendlyByteBuf extraData) {
        this(containerId, playerInventory, null);
    }

    @Override
    public boolean stillValid(Player player) {
        return true;
    }

    @Override
    public ItemStack quickMoveStack(Player player, int index) {
        return ItemStack.EMPTY;
    }

    /**
     * Called when the "Reforge/Socket" button is pressed.
     */
    public void performAction(Player player) {
        ItemStack armor = itemHandler.getStackInSlot(0);
        ItemStack material = itemHandler.getStackInSlot(1);
        ItemStack rune = itemHandler.getStackInSlot(2);

        if (!armor.isEmpty() && !rune.isEmpty() && armor.getItem() instanceof ElysiumArmorItem) {
            // Socket rune
            ElysiumArmorItem armorItem = (ElysiumArmorItem) armor.getItem();
            ElysiumRuneItem runeItem = (ElysiumRuneItem) rune.getItem();
            
            if (armorItem.socketRune(armor, runeItem.getRuneType())) {
                rune.shrink(1);
            }
        } else if (!armor.isEmpty() && !material.isEmpty()) {
            // Reforge armor
            ItemStack result = ElysiumReforgeHandler.reforge(armor, material);
            if (!result.isEmpty()) {
                itemHandler.setStackInSlot(0, result);
                material.shrink(1);
            }
        }
    }
}