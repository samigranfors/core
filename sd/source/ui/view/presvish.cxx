/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <PresentationViewShell.hxx>
#include <optsitem.hxx>
#include <sddll.hxx>
#include <sfx2/request.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/viewfrm.hxx>
#include <svx/svxids.hrc>
#include <svx/ruler.hxx>
#include <FrameView.hxx>
#include <DrawDocShell.hxx>
#include <slideshow.hxx>
#include <sdattr.hxx>
#include <sdpage.hxx>
#include <drawdoc.hxx>
#include <drawview.hxx>
#include <app.hrc>
#include <ViewShellBase.hxx>
#include <FactoryIds.hxx>

#include <fupoor.hxx>
#include <Window.hxx>

#define ShellClass_PresentationViewShell
using namespace sd;
#include <sdslots.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;

namespace sd {

SFX_IMPL_INTERFACE(PresentationViewShell, DrawViewShell)

void PresentationViewShell::InitInterface_Impl()
{
    GetStaticInterface()->RegisterObjectBar(SFX_OBJECTBAR_TOOLS, SfxVisibilityFlags::Standard | SfxVisibilityFlags::FullScreen | SfxVisibilityFlags::Server,
                                            ToolbarId::Draw_Toolbox_Sd);
    GetStaticInterface()->RegisterObjectBar(SFX_OBJECTBAR_APPLICATION, SfxVisibilityFlags::Standard | SfxVisibilityFlags::Client | SfxVisibilityFlags::Viewer | SfxVisibilityFlags::ReadonlyDoc,
                                            ToolbarId::Draw_Viewer_Toolbox);
    GetStaticInterface()->RegisterObjectBar(SFX_OBJECTBAR_OPTIONS, SfxVisibilityFlags::Standard | SfxVisibilityFlags::Server,
                                            ToolbarId::Draw_Options_Toolbox);
    GetStaticInterface()->RegisterObjectBar(SFX_OBJECTBAR_COMMONTASK, SfxVisibilityFlags::Standard | SfxVisibilityFlags::Server,
                                            ToolbarId::Draw_CommonTask_Toolbox);
}


PresentationViewShell::PresentationViewShell( ViewShellBase& rViewShellBase, vcl::Window* pParentWindow, FrameView* pFrameView)
: DrawViewShell( rViewShellBase, pParentWindow, PageKind::Standard, pFrameView)
{
    if( GetDocSh() && GetDocSh()->GetCreateMode() == SfxObjectCreateMode::EMBEDDED )
        maOldVisArea = GetDocSh()->GetVisArea( ASPECT_CONTENT );
    meShellType = ST_PRESENTATION;
}

PresentationViewShell::~PresentationViewShell()
{
    if( GetDocSh() && GetDocSh()->GetCreateMode() == SfxObjectCreateMode::EMBEDDED && !maOldVisArea.IsEmpty() )
        GetDocSh()->SetVisArea( maOldVisArea );
}

void PresentationViewShell::FinishInitialization( FrameView* pFrameView )
{
    DrawViewShell::Init(true);

    // Use the frame view that comes form the view shell that initiated our
    // creation.
    if (pFrameView != nullptr)
    {
        GetFrameView()->Disconnect();
        SetFrameView (pFrameView);
        pFrameView->Connect();
    }
    SetRuler(false);
    WriteFrameViewData();

    GetActiveWindow()->GrabFocus();
}

VclPtr<SvxRuler> PresentationViewShell::CreateHRuler(::sd::Window*)
{
    return nullptr;
}

VclPtr<SvxRuler> PresentationViewShell::CreateVRuler(::sd::Window*)
{
    return nullptr;
}

void PresentationViewShell::Activate( bool bIsMDIActivate )
{
    DrawViewShell::Activate( bIsMDIActivate );

    if( bIsMDIActivate )
    {
        SfxBoolItem aItem( SID_NAVIGATOR_INIT, true );

        GetViewFrame()->GetDispatcher()->ExecuteList(SID_NAVIGATOR_INIT,
                SfxCallMode::ASYNCHRON | SfxCallMode::RECORD, { &aItem });

        rtl::Reference< SlideShow > xSlideShow( SlideShow::GetSlideShow( GetViewShellBase() ) );
        if( xSlideShow.is() )
            xSlideShow->activate(GetViewShellBase());

        if( HasCurrentFunction() )
            GetCurrentFunction()->Activate();
    }

    if( bIsMDIActivate )
        ReadFrameViewData( mpFrameView );
    GetDocSh()->Connect( this );
}

void PresentationViewShell::Paint( const ::tools::Rectangle& /*rRect*/, ::sd::Window* )
{
    rtl::Reference< SlideShow > xSlideShow( SlideShow::GetSlideShow( GetViewShellBase() ) );
    if( xSlideShow.is() )
        xSlideShow->paint();
}

void PresentationViewShell::Resize()
{
    ViewShell::Resize(); // do not call DrawViewShell here!

    rtl::Reference< sd::SlideShow > xSlideshow( SlideShow::GetSlideShow( GetViewShellBase() ) );
    if( xSlideshow.is() )
        xSlideshow->resize(maViewSize);
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
