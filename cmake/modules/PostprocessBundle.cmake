# Adapted from the Dolphin project: https://dolphin-emu.org/

# This module can be used in two different ways.
#
# When invoked as `cmake -P PostprocessBundle.cmake`, it fixes up an
# application folder to be standalone. It bundles all required libraries from
# the system and fixes up library IDs. Any additional shared libraries, like
# plugins, that are found under Contents/MacOS/ will be made standalone as well.
#
# When called with `include(PostprocessBundle)`, it defines a helper
# function `postprocess_bundle` that sets up the command form of the
# module as a post-build step.

if(CMAKE_GENERATOR)
	# Being called as include(PostprocessBundle), so define a helper function.
	set(POSTPROCESS_BUNDLE_MODULE_LOCATION "${CMAKE_CURRENT_LIST_FILE}")
	function(postprocess_bundle target path)
		add_custom_command(TARGET ${target}
			POST_BUILD
			COMMAND ${CMAKE_COMMAND} -D "BUNDLE_PATH=${path}"
				-P "${POSTPROCESS_BUNDLE_MODULE_LOCATION}"
				VERBATIM
		)

		# codesigning is a requirement for apps for Apple Silicon/arm64 devices
		#  so take the simplest road and self-sign with an ad-hoc signature
		if ( APPLE AND CREATE_BUNDLE )
		  add_custom_command(TARGET ${target} POST_BUILD
		      COMMAND codesign --force  --deep -s - "${path}"
		      COMMENT "(macOS) signing \"${path}\" with an ad-hoc signature"
		      VERBATIM
		  )
		endif()
	endfunction()
	return()
endif()

# Make sure to fix up any additional shared libraries (like plugins) that are
# needed.
file(GLOB_RECURSE BUNDLE_LIBS "${BUNDLE_PATH}/Contents/MacOS/*.dylib")

include(BundleUtilities)
set(BU_CHMOD_BUNDLE_ITEMS ON)
fixup_bundle("${BUNDLE_PATH}" "${BUNDLE_LIBS}" "")

if( ENABLE_VERIFY_APP )
	verify_app("${BUNDLE_PATH}")
endif()