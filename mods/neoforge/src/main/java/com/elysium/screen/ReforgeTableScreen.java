package com.elysium.screen;

import com.elysium.Elysium;
import com.elysium.menu.ReforgeTableMenu;
import com.mojang.blaze3d.systems.RenderSystem;
import net.minecraft.client.gui.GuiGraphics;
import net.minecraft.client.gui.screens.inventory.AbstractContainerScreen;
import net.minecraft.client.gui.components.Button;
import net.minecraft.network.chat.Component;
import net.minecraft.resources.ResourceLocation;
import net.minecraft.world.entity.player.Inventory;

public class ReforgeTableScreen extends AbstractContainerScreen<ReforgeTableMenu> {

    private static final ResourceLocation TEXTURE = new ResourceLocation(Elysium.MODID, "textures/gui/reforge_table.png");

    public ReforgeTableScreen(ReforgeTableMenu menu, Inventory playerInventory, Component title) {
        super(menu, playerInventory, title);
        this.imageWidth = 176;
        this.imageHeight = 166;
    }

    @Override
    protected void init() {
        super.init();
        
        // Add Reforge button
        this.addRenderableWidget(Button.builder(Component.literal("Reforge"), button -> {
            if (this.minecraft != null && this.minecraft.player != null) {
                // Send reforge packet or call menu method
                // For now, just close the screen
                this.minecraft.player.closeContainer();
            }
        }).bounds(this.leftPos + 70, this.topPos + 55, 60, 20).build());
    }

    @Override
    protected void renderBg(GuiGraphics guiGraphics, float partialTick, int mouseX, int mouseY) {
        RenderSystem.setShaderTexture(0, TEXTURE);
        int x = (this.width - this.imageWidth) / 2;
        int y = (this.height - this.imageHeight) / 2;
        guiGraphics.blit(TEXTURE, x, y, 0, 0, this.imageWidth, this.imageHeight);
    }

    @Override
    public void render(GuiGraphics guiGraphics, int mouseX, int mouseY, float partialTick) {
        this.renderBackground(guiGraphics);
        super.render(guiGraphics, mouseX, mouseY, partialTick);
        this.renderTooltip(guiGraphics, mouseX, mouseY);
    }
}