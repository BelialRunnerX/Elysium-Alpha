package com.elysium.core.screen;

import com.elysium.core.Elysium;
import com.elysium.core.menu.ReforgeTableMenu;
import net.minecraft.client.gui.GuiGraphics;
import net.minecraft.client.gui.components.Button;
import net.minecraft.client.gui.screens.inventory.AbstractContainerScreen;
import net.minecraft.network.chat.Component;
import net.minecraft.resources.ResourceLocation;
import net.minecraft.world.entity.player.Inventory;

/**
 * Screen for the Elysium workstations.
 *
 * The action button now actually does something: it sends a vanilla container
 * button click, which the server picks up in
 * {@link ReforgeTableMenu#clickMenuButton}. The old version just closed the
 * screen, and its texture was referenced through a constructor that no longer
 * exists.
 */
public class ReforgeTableScreen extends AbstractContainerScreen<ReforgeTableMenu> {

    private static final ResourceLocation TEXTURE =
            ResourceLocation.fromNamespaceAndPath(Elysium.MODID, "textures/gui/reforge_table.png");

    public ReforgeTableScreen(ReforgeTableMenu menu, Inventory playerInventory, Component title) {
        super(menu, playerInventory, title);
        this.imageWidth = 176;
        this.imageHeight = 166;
        this.inventoryLabelY = this.imageHeight - 94;
    }

    @Override
    protected void init() {
        super.init();

        this.addRenderableWidget(Button.builder(
                        Component.translatable("elysium.gui.reforge"),
                        button -> {
                            if (this.minecraft != null && this.minecraft.gameMode != null) {
                                this.minecraft.gameMode.handleInventoryButtonClick(
                                        this.menu.containerId, ReforgeTableMenu.BUTTON_PERFORM);
                            }
                        })
                .bounds(this.leftPos + 58, this.topPos + 56, 60, 20)
                .build());
    }

    @Override
    protected void renderBg(GuiGraphics guiGraphics, float partialTick, int mouseX, int mouseY) {
        guiGraphics.blit(TEXTURE, this.leftPos, this.topPos, 0, 0, this.imageWidth, this.imageHeight);
    }

    @Override
    public void render(GuiGraphics guiGraphics, int mouseX, int mouseY, float partialTick) {
        super.render(guiGraphics, mouseX, mouseY, partialTick);
        this.renderTooltip(guiGraphics, mouseX, mouseY);
    }
}
