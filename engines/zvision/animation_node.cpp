/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/scummsys.h"

#include "zvision/animation_node.h"

#include "zvision/zvision.h"
#include "zvision/render_manager.h"
#include "zvision/script_manager.h"
#include "zvision/rlf_animation.h"
#include "zvision/zork_avi_decoder.h"

#include "video/video_decoder.h"

#include "graphics/surface.h"


namespace ZVision {

AnimationNode::AnimationNode(ZVision *engine, uint32 controlKey, const Common::String &fileName, int32 mask, int32 frate, bool DisposeAfterUse)
	: SideFX(engine, controlKey, SIDEFX_ANIM),
	  _fileType(RLF),
	  _DisposeAfterUse(DisposeAfterUse),
	  _mask(mask) {
	if (fileName.hasSuffix(".rlf")) {
		_fileType = RLF;
		_animation.rlf = new RlfAnimation(fileName, false);
		_frmDelay = _animation.rlf->frameTime();
	} else if (fileName.hasSuffix(".avi")) {
		_fileType = AVI;
		_animation.avi = new ZorkAVIDecoder();
		_animation.avi->loadFile(fileName);
		_frmDelay = 1000.0 / _animation.avi->getDuration().framerate();
	} else {
		warning("Unrecognized animation file type: %s", fileName.c_str());
	}

	if (frate > 0)
		_frmDelay = 1000.0 / frate;
}

AnimationNode::~AnimationNode() {
	if (_fileType == RLF) {
		delete _animation.rlf;
	} else if (_fileType == AVI) {
		delete _animation.avi;
	}

	_engine->getScriptManager()->setStateValue(_key, 2);

	PlayNodes::iterator it = _playList.begin();
	if (it != _playList.end()) {
		_engine->getScriptManager()->setStateValue((*it).slot, 2);

		if ((*it)._scaled)
			delete(*it)._scaled;
	}

	_playList.clear();
}

bool AnimationNode::process(uint32 deltaTimeInMillis) {
	PlayNodes::iterator it = _playList.begin();
	if (it != _playList.end()) {
		playnode *nod = &(*it);

		nod->_delay -= deltaTimeInMillis;
		if (nod->_delay <= 0) {
			nod->_delay += _frmDelay;

			const Graphics::Surface *frame = NULL;

			if (nod->_cur_frm == -1) { // Start of new playlist node
				nod->_cur_frm = nod->start;
				if (_fileType == RLF) {
					_animation.rlf->seekToFrame(nod->_cur_frm);
					frame = _animation.rlf->decodeNextFrame();
				} else if (_fileType == AVI) {
					_animation.avi->seekToFrame(nod->_cur_frm);
					frame = _animation.avi->decodeNextFrame();
				}

				nod->_delay = _frmDelay;
				if (nod->slot)
					_engine->getScriptManager()->setStateValue(nod->slot, 1);
			} else {
				nod->_cur_frm++;

				if (nod->_cur_frm > nod->stop) {
					nod->loop--;

					if (nod->loop == 0) {
						if (nod->slot >= 0)
							_engine->getScriptManager()->setStateValue(nod->slot, 2);
						if (nod->_scaled)
							delete nod->_scaled;
						_playList.erase(it);
						return _DisposeAfterUse;
					}

					nod->_cur_frm = nod->start;
					if (_fileType == RLF) {
						_animation.rlf->seekToFrame(nod->_cur_frm);
						frame = _animation.rlf->decodeNextFrame();
					} else if (_fileType == AVI) {
						_animation.avi->seekToFrame(nod->_cur_frm);
						frame = _animation.avi->decodeNextFrame();
					}
				} else {
					if (_fileType == RLF)
						frame = _animation.rlf->decodeNextFrame();
					else if (_fileType == AVI)
						frame = _animation.avi->decodeNextFrame();
				}
			}

			if (frame) {

				uint32 dstw;
				uint32 dsth;
				if (_engine->getRenderManager()->getRenderTable()->getRenderState() == RenderTable::PANORAMA) {
					dstw = nod->pos.height();
					dsth = nod->pos.width();
				} else {
					dstw = nod->pos.width();
					dsth = nod->pos.height();
				}

				if (frame->w != dstw || frame->h != dsth) {
					if (nod->_scaled)
						if (nod->_scaled->w != dstw || nod->_scaled->h != dsth) {
							delete nod->_scaled;
							nod->_scaled = NULL;
						}

					if (!nod->_scaled) {
						nod->_scaled = new Graphics::Surface;
						nod->_scaled->create(dstw, dsth, frame->format);
					}

					_engine->getRenderManager()->scaleBuffer(frame->getPixels(), nod->_scaled->getPixels(), frame->w, frame->h, frame->format.bytesPerPixel, dstw, dsth);
					frame = nod->_scaled;
				}

				if (_engine->getRenderManager()->getRenderTable()->getRenderState() == RenderTable::PANORAMA) {
					Graphics::Surface *transposed = RenderManager::tranposeSurface(frame);
					if (_mask > 0)
						_engine->getRenderManager()->renderImageToBackground(*transposed, nod->pos.left, nod->pos.top, _mask);
					else
						_engine->getRenderManager()->renderImageToBackground(*transposed, nod->pos.left, nod->pos.top);
					delete transposed;
				} else {
					if (_mask > 0)
						_engine->getRenderManager()->renderImageToBackground(*frame, nod->pos.left, nod->pos.top, _mask);
					else
						_engine->getRenderManager()->renderImageToBackground(*frame, nod->pos.left, nod->pos.top);
				}
			}
		}
	}

	return false;
}



void AnimationNode::addPlayNode(int32 slot, int x, int y, int x2, int y2, int start_frame, int end_frame, int loops) {
	playnode nod;
	nod.loop = loops;
	nod.pos = Common::Rect(x, y, x2 + 1, y2 + 1);
	nod.start = start_frame;
	nod.stop = end_frame;
	nod.slot = slot;
	nod._cur_frm = -1;
	nod._delay = 0;
	nod._scaled = NULL;
	_playList.push_back(nod);
}

bool AnimationNode::stop() {
	PlayNodes::iterator it = _playList.begin();
	if (it != _playList.end()) {
		_engine->getScriptManager()->setStateValue((*it).slot, 2);
		if ((*it)._scaled)
			delete(*it)._scaled;
	}

	_playList.clear();

	// We don't need to delete, it's may be reused
	return false;
}

} // End of namespace ZVision
