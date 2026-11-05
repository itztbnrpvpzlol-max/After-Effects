package com.aftereffects.mobile

import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.compose.foundation.Canvas
import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.clickable
import androidx.compose.foundation.horizontalScroll
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.foundation.text.BasicTextField
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Surface
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.geometry.Offset
import androidx.compose.ui.geometry.Size
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.drawscope.Stroke
import androidx.compose.ui.text.TextStyle
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.style.TextOverflow
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp

private val AppBg = Color(0xFF202020)
private val PanelBg = Color(0xFF242424)
private val PanelAlt = Color(0xFF292929)
private val Border = Color(0xFF414141)
private val TextMain = Color(0xFFBDBDBD)
private val TextMuted = Color(0xFF858585)
private val Blue = Color(0xFF3977B9)
private val TimelineBlue = Color(0xFF526F9C)

class MainActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        window.statusBarColor = android.graphics.Color.rgb(32, 32, 32)
        window.navigationBarColor = android.graphics.Color.rgb(32, 32, 32)
        window.decorView.systemUiVisibility = 0
        setContent {
            MaterialTheme {
                var editor by remember { mutableStateOf(false) }
                var compositionName by remember { mutableStateOf("Comp 1") }
                Surface(Modifier.fillMaxSize(), color = AppBg) {
                    if (editor) EditorWorkspace(compositionName) { editor = false }
                    else HomeWorkspace(
                        onNewComposition = { compositionName = "Comp 1"; editor = true }
                    )
                }
            }
        }
    }
}

@Composable
private fun AeText(
    value: String,
    modifier: Modifier = Modifier,
    color: Color = TextMain,
    size: Int = 12,
    weight: FontWeight = FontWeight.Normal,
    maxLines: Int = 1
) {
    androidx.compose.material3.Text(
        text = value,
        modifier = modifier,
        color = color,
        fontSize = size.sp,
        fontWeight = weight,
        fontFamily = FontFamily.SansSerif,
        maxLines = maxLines,
        overflow = TextOverflow.Ellipsis,
        lineHeight = (size + 3).sp
    )
}

@Composable
private fun Panel(
    modifier: Modifier = Modifier,
    contentPadding: PaddingValues = PaddingValues(9.dp),
    content: @Composable ColumnScope.() -> Unit
) {
    Column(
        modifier
            .background(PanelBg)
            .border(1.dp, Border, RoundedCornerShape(3.dp))
            .padding(contentPadding),
        content = content
    )
}

@Composable
private fun MenuStrip() {
    Row(
        Modifier.fillMaxWidth().height(31.dp)
            .background(Color(0xFF252525))
            .padding(horizontal = 8.dp),
        verticalAlignment = Alignment.CenterVertically
    ) {
        listOf("File", "Edit", "Composition", "Layer", "Effect", "Animation", "View", "Window", "Help")
            .forEach { label ->
                AeText(label, Modifier.padding(end = 13.dp), color = TextMain, size = 11)
            }
        Spacer(Modifier.weight(1f))
        AeText("Search Help", color = TextMuted, size = 10)
    }
}

@Composable
private fun HomeWorkspace(onNewComposition: () -> Unit) {
    Column(Modifier.fillMaxSize().background(AppBg).padding(7.dp)) {
        MenuStrip()
        Spacer(Modifier.height(5.dp))
        Row(Modifier.weight(0.69f).fillMaxWidth(), horizontalArrangement = Arrangement.spacedBy(5.dp)) {
            Panel(Modifier.weight(0.235f).fillMaxHeight()) {
                AeText("Project", size = 13)
                Spacer(Modifier.height(7.dp))
                Box(Modifier.fillMaxWidth().height(1.dp).background(Border))
                Spacer(Modifier.height(12.dp))
                AeText("Search", color = TextMuted, size = 11)
                Spacer(Modifier.height(7.dp))
                Box(Modifier.fillMaxWidth().height(27.dp).background(PanelAlt, RoundedCornerShape(3.dp)))
                Spacer(Modifier.height(13.dp))
                Row(Modifier.fillMaxWidth(), horizontalArrangement = Arrangement.SpaceBetween) {
                    AeText("Name", color = TextMuted, size = 10)
                    AeText("Type", color = TextMuted, size = 10)
                }
                Spacer(Modifier.height(5.dp))
                Box(Modifier.fillMaxWidth().height(1.dp).background(Border))
                Spacer(Modifier.height(8.dp))
                AeText("  Project", size = 11)
            }
            Panel(Modifier.weight(0.605f).fillMaxHeight(), PaddingValues(12.dp)) {
                Row(
                    Modifier.fillMaxSize(),
                    horizontalArrangement = Arrangement.Center,
                    verticalAlignment = Alignment.CenterVertically
                ) {
                    CompositionTile(
                        title = "New Composition",
                        icon = "composition",
                        modifier = Modifier.weight(1f).fillMaxHeight(0.62f),
                        onClick = onNewComposition
                    )
                    Spacer(Modifier.width(18.dp))
                    CompositionTile(
                        title = "New Composition\nFrom Footage",
                        icon = "footage",
                        modifier = Modifier.weight(1f).fillMaxHeight(0.62f),
                        onClick = onNewComposition
                    )
                }
            }
            Column(Modifier.weight(0.16f).fillMaxHeight(), verticalArrangement = Arrangement.spacedBy(5.dp)) {
                SidePanel("Info", Modifier.weight(0.85f))
                SidePanel("Audio", Modifier.weight(0.85f))
                Panel(Modifier.weight(1.35f).fillMaxWidth()) {
                    AeText("Preview", size = 12)
                    Spacer(Modifier.height(8.dp))
                    AeText("◀   ◀   Play   ▶   ▶", color = TextMuted, size = 10)
                }
                SidePanel("Effects & Presets", Modifier.weight(0.85f))
                SidePanel("Align", Modifier.weight(0.75f))
                SidePanel("Libraries", Modifier.weight(0.75f))
            }
        }
        Spacer(Modifier.height(6.dp))
        Panel(Modifier.weight(0.31f).fillMaxWidth(), PaddingValues(9.dp)) {
            AeText("Render Queue", size = 12)
            Spacer(Modifier.height(7.dp))
            Box(Modifier.fillMaxWidth().height(1.dp).background(Border))
            Spacer(Modifier.height(7.dp))
            Row(Modifier.fillMaxWidth()) {
                AeText("Render Settings", Modifier.weight(1f), color = TextMuted, size = 11)
                AeText("Status", Modifier.weight(0.75f), color = TextMuted, size = 11)
                AeText("Output To", Modifier.weight(0.75f), color = TextMuted, size = 11)
            }
        }
    }
}

@Composable
private fun CompositionTile(
    title: String,
    icon: String,
    modifier: Modifier,
    onClick: () -> Unit
) {
    Column(
        modifier
            .background(Color(0xFF202020))
            .border(1.dp, Border, RoundedCornerShape(3.dp))
            .clickable(onClick = onClick)
            .padding(13.dp),
        horizontalAlignment = Alignment.CenterHorizontally
    ) {
        Spacer(Modifier.weight(0.75f))
        Canvas(Modifier.size(width = 78.dp, height = 54.dp)) {
            val stroke = 1.4.dp.toPx()
            val left = size.width * 0.13f
            val top = size.height * 0.10f
            val w = size.width * 0.72f
            val h = size.height * 0.72f
            drawRect(Color(0xFF777777), Offset(left, top), Size(w, h), style = Stroke(stroke))
            if (icon == "footage") {
                drawRect(Color(0xFF777777), Offset(left - 10.dp.toPx(), top + 10.dp.toPx()), Size(19.dp.toPx(), h - 4.dp.toPx()), style = Stroke(stroke))
                for (i in 0..3) {
                    val x = left - 7.dp.toPx() + i * 5.dp.toPx()
                    drawLine(Color(0xFF777777), Offset(x, top + 13.dp.toPx()), Offset(x, top + h - 5.dp.toPx()), stroke)
                }
            }
            drawCircle(Color(0xFF777777), radius = 8.dp.toPx(), center = Offset(left + w * 0.56f, top + h * 0.58f), style = Stroke(stroke))
            drawLine(Color(0xFF777777), Offset(left + w * 0.34f, top + h * 0.72f), Offset(left + w * 0.52f, top + h * 0.38f), stroke)
            drawLine(Color(0xFF777777), Offset(left + w * 0.52f, top + h * 0.38f), Offset(left + w * 0.72f, top + h * 0.72f), stroke)
        }
        Spacer(Modifier.height(17.dp))
        AeText(title, color = Color(0xFFB7B7B7), size = 11, maxLines = 2)
        Spacer(Modifier.weight(0.75f))
    }
}

@Composable
private fun SidePanel(title: String, modifier: Modifier = Modifier) {
    Panel(modifier.fillMaxWidth(), PaddingValues(horizontal = 9.dp, vertical = 7.dp)) {
        AeText(title, size = 11)
    }
}

@Composable
private fun EditorWorkspace(compositionName: String, onBack: () -> Unit) {
    var playhead by remember { mutableFloatStateOf(0.12f) }
    var selectedLayer by remember { mutableIntStateOf(0) }
    Column(Modifier.fillMaxSize().background(AppBg).padding(5.dp)) {
        MenuStrip()
        Spacer(Modifier.height(3.dp))
        Row(
            Modifier.fillMaxWidth().height(34.dp).background(Color(0xFF292929)).padding(horizontal = 5.dp),
            verticalAlignment = Alignment.CenterVertically
        ) {
            listOf("↖", "✋", "⌕", "↻", "▣", "✎", "T", "▱", "◩", "◌").forEach {
                AeText(it, Modifier.padding(horizontal = 7.dp), size = 15)
            }
            Spacer(Modifier.weight(1f))
            AeText("Fill     Stroke  2 px       Essentials", color = TextMuted, size = 10)
        }
        Spacer(Modifier.height(4.dp))
        Row(Modifier.weight(0.57f).fillMaxWidth(), horizontalArrangement = Arrangement.spacedBy(4.dp)) {
            Column(Modifier.weight(0.235f).fillMaxHeight(), verticalArrangement = Arrangement.spacedBy(4.dp)) {
                Panel(Modifier.weight(0.62f).fillMaxWidth()) {
                    Row(verticalAlignment = Alignment.CenterVertically) {
                        AeText("Project", size = 11)
                        Spacer(Modifier.width(12.dp))
                        AeText("Effect Controls", color = TextMuted, size = 10)
                    }
                    Spacer(Modifier.height(8.dp))
                    Box(Modifier.fillMaxWidth().height(22.dp).background(PanelAlt))
                    Spacer(Modifier.height(8.dp))
                    AeText("Name                    Type", color = TextMuted, size = 9)
                    AeText(compositionName, size = 10)
                }
                Panel(Modifier.weight(0.38f).fillMaxWidth()) {
                    AeText("Effect Controls", size = 11)
                }
            }
            Panel(Modifier.weight(0.605f).fillMaxHeight(), PaddingValues(6.dp)) {
                Row(Modifier.fillMaxWidth(), verticalAlignment = Alignment.CenterVertically) {
                    AeText("Composition", size = 11)
                    AeText("  $compositionName", color = Blue, size = 11)
                    Spacer(Modifier.weight(1f))
                    AeText("50%   Full   Active Camera   1 View", color = TextMuted, size = 9)
                }
                Spacer(Modifier.height(5.dp))
                Box(Modifier.fillMaxSize().background(Color.Black), contentAlignment = Alignment.Center) {
                    Canvas(Modifier.fillMaxSize()) {
                        val r = size.width * 0.055f
                        drawCircle(Color(0xFFD5F21D), r, Offset(size.width * 0.20f, size.height * 0.32f))
                        drawCircle(Color.LightGray, r, Offset(size.width * 0.20f, size.height * 0.32f), style = Stroke(2.dp.toPx()))
                        drawRect(Color(0xFFD5F21D), Offset(size.width * 0.78f, size.height * 0.46f), Size(size.width * 0.10f, size.height * 0.26f))
                        drawRect(Color.LightGray, Offset(size.width * 0.78f, size.height * 0.46f), Size(size.width * 0.10f, size.height * 0.26f), style = Stroke(2.dp.toPx()))
                    }
                }
            }
            Column(Modifier.weight(0.16f).fillMaxHeight(), verticalArrangement = Arrangement.spacedBy(4.dp)) {
                SidePanel("Info", Modifier.weight(0.8f))
                SidePanel("Audio", Modifier.weight(0.8f))
                Panel(Modifier.weight(1.3f).fillMaxWidth()) {
                    AeText("Preview", size = 11)
                    Spacer(Modifier.height(7.dp))
                    AeText("◀  ◀  Play  ▶  ▶", color = TextMuted, size = 9)
                    AeText("Shortcut: Spacebar", color = TextMuted, size = 9)
                }
                SidePanel("Effects & Presets", Modifier.weight(0.9f))
                SidePanel("Align", Modifier.weight(0.8f))
            }
        }
        Spacer(Modifier.height(4.dp))
        Panel(Modifier.weight(0.43f).fillMaxWidth(), PaddingValues(5.dp)) {
            Row(Modifier.fillMaxWidth().height(24.dp), verticalAlignment = Alignment.CenterVertically) {
                AeText(compositionName, size = 11)
                Spacer(Modifier.width(12.dp))
                AeText("Render Queue", color = TextMuted, size = 10)
                Spacer(Modifier.weight(1f))
                AeText("▶   Add Layer  +", color = TextMuted, size = 10)
            }
            Box(Modifier.fillMaxWidth().height(1.dp).background(Border))
            Row(Modifier.fillMaxSize()) {
                Column(Modifier.fillMaxHeight().weight(0.25f)) {
                    AeText("Source Name                  Mode", color = TextMuted, size = 9)
                    listOf("Shape Layer 2", "Shape Layer 1", "Background").forEachIndexed { index, name ->
                        Row(
                            Modifier.fillMaxWidth().height(35.dp)
                                .background(if (selectedLayer == index) Color(0xFF343B49) else Color.Transparent)
                                .clickable { selectedLayer = index },
                            verticalAlignment = Alignment.CenterVertically
                        ) { AeText("  $name", size = 10) }
                    }
                }
                Column(Modifier.weight(0.75f).fillMaxHeight()) {
                    Canvas(
                        Modifier.fillMaxSize().horizontalScroll(rememberScrollState())
                    ) {
                        val ruler = 25.dp.toPx()
                        drawRect(Color(0xFF282828), Offset.Zero, Size(size.width, ruler))
                        for (i in 0..12) {
                            val x = size.width * i / 12f
                            drawLine(Color(0xFF555555), Offset(x, 0f), Offset(x, ruler), 1.dp.toPx())
                        }
                        for (row in 0..2) {
                            val y = ruler + 7.dp.toPx() + row * 37.dp.toPx()
                            drawLine(Color(0xFF484848), Offset(0f, y + 32.dp.toPx()), Offset(size.width, y + 32.dp.toPx()), 1.dp.toPx())
                            drawRect(
                                if (row == selectedLayer) TimelineBlue else Color(0xFF46546C),
                                Offset(3.dp.toPx(), y + 3.dp.toPx()),
                                Size(size.width * (0.94f - row * 0.04f), 25.dp.toPx())
                            )
                            for (key in 0..(if (row == 0) 2 else 1)) {
                                val x = size.width * (0.02f + key * 0.13f)
                                val p = PathKey(x, y + 15.dp.toPx(), 5.dp.toPx())
                                drawLine(Color(0xFFE5C16B), p.first, p.second, 2.dp.toPx())
                                drawLine(Color(0xFFE5C16B), p.second, p.third, 2.dp.toPx())
                                drawLine(Color(0xFFE5C16B), p.third, p.fourth, 2.dp.toPx())
                                drawLine(Color(0xFFE5C16B), p.fourth, p.first, 2.dp.toPx())
                            }
                        }
                        val px = size.width * playhead
                        drawLine(Color(0xFF62A0F0), Offset(px, 0f), Offset(px, size.height), 2.dp.toPx())
                    }
                }
            }
        }
    }
}

private data class KeyDiamond(val first: Offset, val second: Offset, val third: Offset, val fourth: Offset)
private fun PathKey(x: Float, y: Float, r: Float) = KeyDiamond(
    Offset(x, y-r), Offset(x+r, y), Offset(x, y+r), Offset(x-r, y)
)
