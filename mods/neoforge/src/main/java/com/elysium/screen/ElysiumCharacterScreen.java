package com.elysium.screen;

import com.elysium.character.ElysiumClass;
import com.elysium.character.ElysiumRace;
import com.elysium.network.CharacterSheet;
import com.elysium.network.ElysiumPayloads;
import com.elysium.stats.ElysiumStat;
import net.minecraft.ChatFormatting;
import net.minecraft.client.gui.GuiGraphics;
import net.minecraft.client.gui.components.Button;
import net.minecraft.client.gui.screens.Screen;
import net.minecraft.network.chat.Component;
import net.neoforged.neoforge.network.PacketDistributor;

/**
 * The character screen: a picker on first join, a sheet thereafter.
 *
 * <h2>Two modes, one screen</h2>
 *
 * A player who has not chosen sees six races and nine classes and cannot leave
 * until they pick — {@link #shouldCloseOnEsc()} returns false and there is no
 * cancel button, because a character with no race is a character with no stats
 * and every other system would have to carry a null check forever.
 *
 * A player who has chosen sees what they are, what they have, and a row of
 * buttons for spending free points. Same screen, because it is the same
 * question asked twice: what is this character.
 *
 * <h2>Why it draws its own list rather than using a scroll widget</h2>
 *
 * Fifteen buttons fit on a screen. A list widget would be the right answer at
 * fifty, and the wrong kind of complexity at fifteen.
 */
public class ElysiumCharacterScreen extends Screen {

    private static final int PANEL = 0xC0101018;
    private static final int PANEL_EDGE = 0xFF413876;
    private static final int SELECTED = 0xFF7F70C4;

    private final CharacterSheet.Parsed sheet;
    private final int unspent;

    private ElysiumRace pickedRace;
    private ElysiumClass pickedClass;

    /** What the description pane is currently describing. */
    private Component focusTitle = Component.empty();
    private Component focusBody = Component.empty();

    public ElysiumCharacterScreen(CharacterSheet.Parsed sheet, int unspent) {
        super(Component.translatable("elysium.screen.character"));
        this.sheet = sheet;
        this.unspent = unspent;
        this.pickedRace = sheet.race();
        this.pickedClass = sheet.job();
    }

    /**
     * The world keeps running behind this screen.
     *
     * A pause screen in single player would stop the tick that granted the
     * level that opened it, and in multiplayer pausing is not on offer anyway —
     * better that both behave the same.
     */
    @Override
    public boolean isPauseScreen() {
        return false;
    }

    @Override
    public boolean shouldCloseOnEsc() {
        return sheet.chosen();
    }

    // ------------------------------------------------------------------

    @Override
    protected void init() {
        if (!sheet.chosen()) {
            initPicker();
        } else {
            initSheet();
        }
    }

    private void initPicker() {
        int left = this.width / 2 - 210;
        int top = 46;

        for (int i = 0; i < ElysiumRace.values().length; i++) {
            ElysiumRace race = ElysiumRace.values()[i];
            addRenderableWidget(Button.builder(race.getDisplayName(), button -> {
                        this.pickedRace = race;
                        focus(race.getDisplayName(), race.getDescription(),
                                race.getPassiveName(), race.getPassiveDescription());
                    })
                    .bounds(left, top + i * 22, 130, 20)
                    .build());
        }

        int right = this.width / 2 + 80;
        for (int i = 0; i < ElysiumClass.values().length; i++) {
            ElysiumClass job = ElysiumClass.values()[i];
            addRenderableWidget(Button.builder(job.getDisplayName(), button -> {
                        this.pickedClass = job;
                        focus(job.getDisplayName(), job.getDescription(),
                                job.getPassiveName(), job.getPassiveDescription());
                    })
                    .bounds(right, top + i * 22, 130, 20)
                    .build());
        }

        addRenderableWidget(Button.builder(
                        Component.translatable("elysium.screen.confirm"), button -> confirm())
                .bounds(this.width / 2 - 60, this.height - 34, 120, 20)
                .build());
    }

    private void initSheet() {
        int left = this.width / 2 - 150;
        int top = 60;

        // One "+" per stat, live only while there are points to spend.
        for (int i = 0; i < ElysiumStat.values().length; i++) {
            ElysiumStat stat = ElysiumStat.values()[i];
            int column = i / 6;
            int row = i % 6;
            Button plus = Button.builder(Component.literal("+"), button -> spend(stat))
                    .bounds(left + column * 160 + 130, top + row * 22, 20, 20)
                    .build();
            plus.active = unspent > 0;
            addRenderableWidget(plus);
        }

        addRenderableWidget(Button.builder(
                        Component.translatable("gui.done"), button -> onClose())
                .bounds(this.width / 2 - 60, this.height - 34, 120, 20)
                .build());
    }

    private void focus(Component title, Component body, Component passive, Component detail) {
        this.focusTitle = title;
        this.focusBody = Component.empty()
                .append(body)
                .append(Component.literal("  "))
                .append(passive)
                .append(Component.literal(" — ").withStyle(ChatFormatting.DARK_GRAY))
                .append(detail);
    }

    private void confirm() {
        if (pickedRace == null || pickedClass == null) {
            // Nothing rude about it — the confirm button simply does nothing
            // until both halves are answered.
            return;
        }
        PacketDistributor.sendToServer(
                new ElysiumPayloads.ChooseCharacter(pickedRace.getId(), pickedClass.getId()));
        onClose();
    }

    private void spend(ElysiumStat stat) {
        PacketDistributor.sendToServer(new ElysiumPayloads.SpendPoints(stat.getId(), 1));
        // The server answers with a fresh sheet, which replaces this screen.
        // Nothing is predicted locally: a stat that briefly reads one point
        // high because the packet was refused is worse than a screen that
        // updates a tick late.
    }

    // ------------------------------------------------------------------

    @Override
    public void render(GuiGraphics graphics, int mouseX, int mouseY, float partialTick) {
        renderBackground(graphics, mouseX, mouseY, partialTick);
        super.render(graphics, mouseX, mouseY, partialTick);

        graphics.drawCenteredString(this.font, this.title, this.width / 2, 16, 0xFFE3D2FF);

        if (!sheet.chosen()) {
            renderPicker(graphics);
        } else {
            renderSheet(graphics);
        }
    }

    private void renderPicker(GuiGraphics graphics) {
        graphics.drawCenteredString(this.font,
                Component.translatable("elysium.screen.choose").withStyle(ChatFormatting.GRAY),
                this.width / 2, 30, 0xFF9E9EB0);

        int paneTop = this.height - 96;
        panel(graphics, this.width / 2 - 210, paneTop, 420, 52);

        graphics.drawString(this.font, focusTitle, this.width / 2 - 200, paneTop + 8, 0xFFE3D2FF);
        graphics.drawWordWrap(this.font, focusBody,
                this.width / 2 - 200, paneTop + 22, 400, 0xFF9E9EB0);

        String picked = (pickedRace == null ? "—" : pickedRace.getDisplayName().getString())
                + "  ·  "
                + (pickedClass == null ? "—" : pickedClass.getDisplayName().getString());
        graphics.drawCenteredString(this.font, picked, this.width / 2, this.height - 48, 0xFFFFD37F);
    }

    private void renderSheet(GuiGraphics graphics) {
        int left = this.width / 2 - 150;
        int top = 60;

        String header = sheet.race().getDisplayName().getString()
                + "  ·  " + sheet.job().getDisplayName().getString()
                + "  ·  " + Component.translatable("elysium.screen.level", sheet.level()).getString();
        graphics.drawCenteredString(this.font, header, this.width / 2, 34, 0xFFFFD37F);

        String progress = sheet.xp() + " / " + sheet.xpNext();
        graphics.drawCenteredString(this.font, progress, this.width / 2, 46, 0xFF6F6F84);

        for (int i = 0; i < ElysiumStat.values().length; i++) {
            ElysiumStat stat = ElysiumStat.values()[i];
            int column = i / 6;
            int row = i % 6;
            int x = left + column * 160;
            int y = top + row * 22 + 6;

            graphics.drawString(this.font, stat.getDisplayName(), x, y, 0xFFC6D0DD);
            graphics.drawString(this.font, Integer.toString(sheet.get(stat)), x + 106, y, 0xFFFFFFFF);
        }

        Component points = Component.translatable("elysium.screen.points", unspent)
                .withStyle(unspent > 0 ? ChatFormatting.GOLD : ChatFormatting.DARK_GRAY);
        graphics.drawCenteredString(this.font, points, this.width / 2, this.height - 52, 0xFFFFD37F);
    }

    /** A filled rectangle with a lit edge, in the mod's own palette. */
    private void panel(GuiGraphics graphics, int x, int y, int width, int height) {
        graphics.fill(x, y, x + width, y + height, PANEL);
        graphics.fill(x, y, x + width, y + 1, PANEL_EDGE);
        graphics.fill(x, y + height - 1, x + width, y + height, SELECTED);
    }
}
