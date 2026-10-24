/*
*	Part of the Oxygen Engine / Sonic 3 A.I.R. software distribution.
*	Copyright (C) 2017-2026 by Eukaryot
*
*	Published under the GNU GPLv3 open source software license, see license.txt
*	or https://www.gnu.org/licenses/gpl-3.0.en.html
*/

#include "oxygen/pch.h"
#include "oxygen/rendering/parts/PlaneManager.h"
#include "oxygen/rendering/parts/PatternManager.h"
#include "oxygen/engine/EngineMain.h"
#include "oxygen/simulation/EmulatorInterface.h"


namespace
{
	// Co-op planes use a fixed 64x32 pattern ring buffer (512x256 pixels), see RenderPlaneShader
	static constexpr int COOP_BUF_W = 64;
	static constexpr int COOP_BUF_H = 32;

	// S3K level data in RAM
	static constexpr uint32 LAYOUT_HEADER   = 0xffff8000;	// u16 widths (FG, BG) and heights (FG, BG) in chunks
	static constexpr uint32 LAYOUT_ROW_PTRS = 0xffff8008;	// Per chunk row: u16 FG row pointer, u16 BG row pointer
	static constexpr uint32 CHUNK_TABLE     = 0xffff0000;	// 128x128 chunks, 8x8 block words each (0x80 bytes)
	static constexpr uint32 BLOCK_TABLE     = 0xffff9000;	// 16x16 blocks, 2x2 pattern words each (8 bytes)

	inline int wrapIndex(int value, int size)
	{
		return (size > 0) ? (((value % size) + size) % size) : 0;
	}

	// Fills the ring buffer with the level layout around the camera, at the same ring positions the VDP name tables use,
	// so the plane is drawn with the camera position as scroll offset just like the standard planes
	void fillCoopBufferFromLevelLayout(uint16* buffer, bool foreground, const Vec2i& cameraPosition)
	{
		EmulatorInterface& emulatorInterface = EmulatorInterface::instance();
		const int layerOffset = foreground ? 0 : 2;
		const int layoutWidth  = emulatorInterface.readMemory16(LAYOUT_HEADER + layerOffset);
		const int layoutHeight = emulatorInterface.readMemory16(LAYOUT_HEADER + 4 + layerOffset);

		// The whole ring buffer is filled (32x16 blocks), starting slightly above/left of the camera
		const int firstBlockX = (cameraPosition.x >> 4) - 2;
		const int firstBlockY = (cameraPosition.y >> 4) - 2;

		for (int by = 0; by < COOP_BUF_H / 2; ++by)
		{
			const int blockY = firstBlockY + by;
			const int chunkRow = wrapIndex(blockY >> 3, layoutHeight);
			const uint32 rowAddress = CHUNK_TABLE + emulatorInterface.readMemory16(LAYOUT_ROW_PTRS + chunkRow * 4 + layerOffset);
			const int py0 = (blockY * 2) & (COOP_BUF_H - 1);
			const int py1 = (py0 + 1) & (COOP_BUF_H - 1);

			for (int bx = 0; bx < COOP_BUF_W / 2; ++bx)
			{
				const int blockX = firstBlockX + bx;
				const int chunkColumn = wrapIndex(blockX >> 3, layoutWidth);
				const uint8 chunkIndex = emulatorInterface.readMemory8(rowAddress + chunkColumn);

				const uint16 blockWord = emulatorInterface.readMemory16(CHUNK_TABLE + chunkIndex * 0x80 + ((blockY & 7) * 8 + (blockX & 7)) * 2);
				const uint32 blockAddress = BLOCK_TABLE + (blockWord & 0x03ff) * 8;
				const bool flipX = (blockWord & 0x0400) != 0;
				const bool flipY = (blockWord & 0x0800) != 0;
				const uint16 flipMask = (uint16)((blockWord & 0x0c00) << 1);	// Block flip bits -> pattern flip bits (0x0800 / 0x1000)
				const uint32 mx = flipX ? 2 : 0;
				const uint32 my = flipY ? 4 : 0;

				const int px0 = (blockX * 2) & (COOP_BUF_W - 1);
				const int px1 = (px0 + 1) & (COOP_BUF_W - 1);

				buffer[py0 * COOP_BUF_W + px0] = emulatorInterface.readMemory16(blockAddress     + mx + my) ^ flipMask;
				buffer[py0 * COOP_BUF_W + px1] = emulatorInterface.readMemory16(blockAddress + 2 - mx + my) ^ flipMask;
				buffer[py1 * COOP_BUF_W + px0] = emulatorInterface.readMemory16(blockAddress + 4 + mx - my) ^ flipMask;
				buffer[py1 * COOP_BUF_W + px1] = emulatorInterface.readMemory16(blockAddress + 6 - mx - my) ^ flipMask;
			}
		}
	}
}



PlaneManager::PlaneManager(PatternManager& patternManager) :
	mPatternManager(patternManager),
	mPlayfieldSize(64, 32)
{
}

void PlaneManager::reset()
{
	mNameTableBaseA = 0xc000;
	mNameTableBaseB = 0xe000;
	mNameTableBaseW = 0x8000;

	mPlayfieldSize.set(64, 32);
	mIsPlaneWBelowSplitY = false;
	mPlaneAWSplitY = 0;

	resetCustomPlanes();
}

void PlaneManager::refresh()
{
	// Build plane pattern textures
	const bool isDeveloperMode = EngineMain::getDelegate().useDeveloperFeatures();
	const int numPatterns = mPlayfieldSize.x * mPlayfieldSize.y;
	for (int index = 0; index < 4; ++index)
	{
		uint16* buffer = mPlanePatternsBuffer[index];
		switch (index)
		{
			case PLANE_DEBUG:
			{
				if (isDeveloperMode)
				{
					for (uint16 k = 0; k < 0x800; ++k)
					{
						buffer[k] = k + ((uint16)mPatternManager.getLastUsedAtex(k) << 9);
					}
				}
				break;
			}

			default:
			{
				if (isPlaneUsed(index))
				{
					const uint16* src = getPlaneContent(index);
					memcpy(buffer, src, numPatterns * sizeof(uint16));
					if (isDeveloperMode)
					{
						for (int k = 0; k < numPatterns; ++k)
						{
							mPatternManager.setLastUsedAtex(src[k], (src[k] >> 9) & 0x70);
						}
					}
				}
				break;
			}
		}
	}
}

void PlaneManager::resetCustomPlanes()
{
	for (int i = 0; i < 4; ++i)
		mDisabledDefaultPlane[i] = false;
	mCustomPlanes.clear();
}

bool PlaneManager::isPlaneUsed(int index) const
{
	// Is index valid at all?
	if (EngineMain::getDelegate().useDeveloperFeatures())
	{
		if (index > PLANE_DEBUG)
			return false;
	}
	else
	{
		if (index >= PLANE_DEBUG)
			return false;
	}

	// Plane A or W may be unused
	if (mPlaneAWSplitY == 0)
	{
		if (mIsPlaneWBelowSplitY)
		{
			if (index == PLANE_A)
				return false;
		}
		else
		{
			if (index == PLANE_W)
				return false;
		}
	}

	return true;
}

Vec2i PlaneManager::getPlayfieldSizeInPatterns() const
{
	return mPlayfieldSize;
}

Vec2i PlaneManager::getPlayfieldSizeInPixels() const
{
	const Vec2i playfieldSize = getPlayfieldSizeInPatterns();
	return Vec2i(playfieldSize.x * 8, playfieldSize.y * 8);
}

Vec4i PlaneManager::getPlayfieldSizeForShaders() const
{
	const Vec2i playfieldSize = getPlayfieldSizeInPatterns();
	return Vec4i(playfieldSize.x * 8, playfieldSize.y * 8, playfieldSize.x, playfieldSize.y);
}

void PlaneManager::setPlayfieldSizeInPatterns(const Vec2i& size)
{
	mPlayfieldSize = size;
}

void PlaneManager::setPlayfieldSizeInPixels(const Vec2i& size)
{
	setPlayfieldSizeInPatterns(size / 8);
}

const uint16* PlaneManager::getPlanePatternsBuffer(uint8 index) const
{
	RMX_ASSERT(index < PLANE_COUNT, "Invalid plane index " << index);
	return mPlanePatternsBuffer[index];
}

uint16 PlaneManager::getPlaneBaseVRAMAddress(int planeIndex) const
{
	switch (planeIndex)
	{
		case PLANE_B:  return mNameTableBaseB;
		case PLANE_A:  return mNameTableBaseA;
		case PLANE_W:  return mNameTableBaseW;
	}
	RMX_ERROR("Invalid plane index", RMX_REACT_THROW);
	return 0;
}

const uint16* PlaneManager::getPlaneDataInVRAM(int planeIndex) const
{
	// Co-op planes are not backed by VRAM; they live in the pre-filled pattern buffer.
	if (planeIndex >= PLANE_COOP_B)
		return mPlanePatternsBuffer[planeIndex];
	return (const uint16*)(EmulatorInterface::instance().getVRam() + getPlaneBaseVRAMAddress(planeIndex));
}

size_t PlaneManager::getPlaneSizeInVRAM(int planeIndex) const
{
	return (size_t)(mPlayfieldSize.x * mPlayfieldSize.y * 2);
}

uint16 PlaneManager::getPatternVRAMAddress(int planeIndex, uint16 patternIndex) const
{
	return getPlaneBaseVRAMAddress(planeIndex) + patternIndex * 2;
}

uint16 PlaneManager::getPatternAtIndex(int planeIndex, uint16 patternIndex) const
{
	if (EngineMain::getDelegate().useDeveloperFeatures())
	{
		if (planeIndex == PLANE_DEBUG)
			return patternIndex;
	}
	return *getPlaneContent(planeIndex, patternIndex);
}

void PlaneManager::setPatternAtIndex(int planeIndex, uint16 patternIndex, uint16 value)
{
	EmulatorInterface::instance().writeVRam16(getPatternVRAMAddress(planeIndex, patternIndex), value);
}

const uint16* PlaneManager::getPlaneContent(int planeIndex, uint16 patternIndex) const
{
	return (uint16*)(EmulatorInterface::instance().getVRam() + getPatternVRAMAddress(planeIndex, patternIndex));
}

void PlaneManager::setWindowPlaneSplitX(bool rightSideWindow, uint16 splitX)
{
	mIsPlaneWRightOfSplitX = rightSideWindow;
	mPlaneAWSplitX = splitX;
}

void PlaneManager::setWindowPlaneSplitY(bool bottomWindow, uint16 splitY)
{
	mIsPlaneWBelowSplitY = bottomWindow;
	mPlaneAWSplitY = splitY;
}

void PlaneManager::setRenderPlaneABehindW(bool renderPlaneABehindW)
{
	mRenderPlaneABehindW = renderPlaneABehindW;
}

void PlaneManager::getPlaneRects(std::vector<PlaneRect>& output, const Recti& fullscreenRect) const
{
	output.clear();

	const int splitX = clamp(mPlaneAWSplitX, 0, fullscreenRect.width);
	const int splitY = clamp(mPlaneAWSplitY, 0, fullscreenRect.height);

	Recti rectA = fullscreenRect;	// Rectangle where plane A is rendered (unless it's enforced to be fullscreen in any case)
	Recti invA;						// Component-wise "inverse" of that rectangle: includes exactly the remaining intervals in both x and y directions individually
	{
		if (mIsPlaneWRightOfSplitX)
		{
			rectA.width = splitX;
			invA.x = splitX;
			invA.width = fullscreenRect.width - splitX;
		}
		else
		{
			invA.width = splitX;
			rectA.x = splitX;
			rectA.width = fullscreenRect.width - splitX;
		}

		if (mIsPlaneWBelowSplitY)
		{
			rectA.height = splitY;
			invA.y = splitY;
			invA.height = fullscreenRect.height - splitY;
		}
		else
		{
			invA.height = splitY;
			rectA.y = splitY;
			rectA.height = fullscreenRect.height - splitY;
		}
	}

	// Plane B
	PlaneRect& planeRectB = vectorAdd(output);
	planeRectB.mPlane = PLANE_B;
	planeRectB.mRect = fullscreenRect;

	if (mRenderPlaneABehindW || !rectA.isEmpty())
	{
		// Plane A
		PlaneRect& planeRectA = vectorAdd(output);
		planeRectA.mPlane = PLANE_A;
		planeRectA.mRect = mRenderPlaneABehindW ? fullscreenRect : rectA;
	}

	if (rectA != fullscreenRect)
	{
		// Plane W
		PlaneRect& planeRectW = vectorAdd(output);
		planeRectW.mPlane = PLANE_W;
		planeRectW.mRect = fullscreenRect;

		if (rectA.isEmpty())
		{
			// Plane W is fullscreen
		}
		else if (rectA.width == fullscreenRect.width)
		{
			// Plane W is full width, but limited height
			planeRectW.mRect.y = invA.y;
			planeRectW.mRect.height = invA.height;
		}
		else if (rectA.height == fullscreenRect.height)
		{
			// Plane W is full height, but limited width
			planeRectW.mRect.x = invA.x;
			planeRectW.mRect.width = invA.width;
		}
		else
		{
			// The screen is split both horizontally and vertically, and three of the four areas are covered by plane W
			PlaneRect& planeRectW2 = vectorAdd(output);
			planeRectW2.mPlane = PLANE_W;
			planeRectW2.mRect = fullscreenRect;

			if (rectA.y == 0)
			{
				// Upper part is limited width
				planeRectW.mRect.x = invA.x;
				planeRectW.mRect.width = invA.width;
				planeRectW.mRect.height = rectA.height;

				// Lower part is full width
				planeRectW2.mRect.y = invA.y;
				planeRectW2.mRect.height = invA.height;
			}
			else
			{
				// Upper part is full width
				planeRectW.mRect.height = invA.height;

				// Lower part is limited width
				planeRectW2.mRect.x = invA.x;
				planeRectW2.mRect.width = invA.width;
				planeRectW2.mRect.y = rectA.y;
				planeRectW2.mRect.height = rectA.height;
			}
		}
	}
}

void PlaneManager::dumpAsPaletteBitmap(PaletteBitmap& output, int planeIndex, bool highlightPrioPatterns) const
{
	Vec2i bitmapSize;
	if (planeIndex <= PLANE_A)
	{
		bitmapSize = getPlayfieldSizeInPixels();
	}
	else
	{
		bitmapSize.set(512, 256);
	}
	output.create(bitmapSize.x, bitmapSize.y);

	const PatternManager::CacheItem* patternCache = mPatternManager.getPatternCache();
	const uint16 numPatternsPerLine = (uint16)(bitmapSize.x / 8);

	uint8* dest = output.getData();
	for (int y = 0; y < bitmapSize.y; ++y)
	{
		for (int x = 0; x < bitmapSize.x; x += 8, dest += 8)
		{
			const uint16 patternIndex = getPatternAtIndex(planeIndex, (x / 8) + (y / 8) * numPatternsPerLine);
			const PatternManager::CacheItem::Pattern& pattern = patternCache[patternIndex & 0x07ff].mFlipVariation[(patternIndex >> 11) & 3];
			const uint8* srcPatternPixels = &pattern.mPixels[(x & 0x07) + (y & 0x07) * 8];
			const uint8 atex = ((planeIndex != PLANE_DEBUG) ? (patternIndex >> 9) : mPatternManager.getLastUsedAtex(patternIndex)) & 0x30;

			for (int k = 0; k < 8; ++k)
			{
				const uint8 colorIndex = srcPatternPixels[k];
				dest[k] = colorIndex + atex;
			}

			const bool lowerBrightness = (highlightPrioPatterns && (patternIndex & 0x8000) == 0);
			if (lowerBrightness)
			{
				for (int k = 0; k < 8; ++k)
				{
					dest[k] |= 0x80;
				}
			}
		}
	}
}

void PlaneManager::setDefaultPlaneEnabled(uint8 index, bool enabled)
{
	mDisabledDefaultPlane[index] = !enabled;
}

void PlaneManager::setupCustomPlane(const Recti& rect, uint8 sourcePlane, uint8 scrollOffsets, uint16 renderQueue)
{
	CustomPlane& plane = vectorAdd(mCustomPlanes);
	plane.mRect = rect;
	plane.mSourcePlane = sourcePlane;
	plane.mScrollOffsets = scrollOffsets;
	plane.mRenderQueue = renderQueue;
}

void PlaneManager::serializeSaveState(VectorBinarySerializer& serializer, uint8 formatVersion)
{
	serializer.serialize(mNameTableBaseA);
	serializer.serialize(mNameTableBaseB);

	if (serializer.isReading())
	{
		Vec2i playfieldSize;
		playfieldSize.x = serializer.read<uint16>();
		playfieldSize.y = serializer.read<uint16>();
		setPlayfieldSizeInPixels(playfieldSize);
	}
	else
	{
		const Vec2i playfieldSize = getPlayfieldSizeInPixels();
		serializer.write<uint16>(playfieldSize.x);
		serializer.write<uint16>(playfieldSize.y);
	}

	if (formatVersion >= 4)
	{
		serializer.serialize(mNameTableBaseW);
		serializer.serializeAs<uint8>(mIsPlaneWBelowSplitY);
		serializer.serialize(mPlaneAWSplitY);

		if (formatVersion >= 7)
		{
			serializer.serializeAs<uint8>(mRenderPlaneABehindW);
		}

		for (int k = 0; k < 4; ++k)
		{
			serializer.serialize(mDisabledDefaultPlane[k]);
		}

		serializer.serializeArraySize(mCustomPlanes, 64);
		for (CustomPlane& customPlane : mCustomPlanes)
		{
			serializer.serializeAs<int16>(customPlane.mRect.x);
			serializer.serializeAs<int16>(customPlane.mRect.y);
			serializer.serializeAs<int16>(customPlane.mRect.width);
			serializer.serializeAs<int16>(customPlane.mRect.height);
			serializer.serialize(customPlane.mSourcePlane);
			serializer.serialize(customPlane.mScrollOffsets);
			serializer.serialize(customPlane.mRenderQueue);
		}
	}
}

void PlaneManager::fillCoopPlane(PlaneType planeType, int cameraX, int cameraY)
{
	RMX_ASSERT(planeType == PLANE_COOP_B || planeType == PLANE_COOP_A, "fillCoopPlane called with non-coop plane index " << planeType);
	fillCoopBufferFromLevelLayout(mPlanePatternsBuffer[planeType], planeType == PLANE_COOP_A, Vec2i(cameraX, cameraY));
}
