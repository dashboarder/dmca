/*
 * Copyright (C) 2014 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#ifndef __LIB_IMAGE4_SPI_H
#define __LIB_IMAGE4_SPI_H


/*
 * An image of type 'image_type' is being loaded.
 *
 * Return true if you'd like its properties captured, false otherwise.
 */
typedef bool (*image4_start_capture_callback)(uint32_t image_type);

/*
 * Property callbacks for supported property types:
 *
 *     tag: 32-bit property tag (e.g. 'EPRO')
 *     is_object_property: true for object property, false for manifest property
 *
 * Note that for the string variant, 'buffer' is only valid within the scope of the callback.
 */
typedef void (*image4_boolean_property_callback)(uint32_t tag, bool is_object_property, bool val);
typedef void (*image4_integer_property_callback)(uint32_t tag, bool is_object_property, uint64_t val);
typedef void (*image4_string_property_callback)(uint32_t tag, bool is_object_property, uint8_t* buffer, size_t length);


/*
 * Validity callback:
 *
 *   true: Image has been validated -- captured properties can be used
 *   false: Image is not valid. You should zero out any previously captured properties!
 */
typedef void (*image4_validity_callback)(bool valid);

/*
 * image4_register_property_capture_callbacks
 *
 * On some targets, we would like to stash away various image4 properties for a particular payload type.
 * Register callbacks for property captures, as well as a callback to let you know whether or not the image ended
 * up being valid.
 *
 * Note for security reasons, you are expected to zero out your stored properties if you find out that
 * image4_validity_callback is called with false.
 */
extern void
image4_register_property_capture_callbacks(image4_start_capture_callback start_cb, image4_boolean_property_callback bool_cb, 
			image4_integer_property_callback int_cb, image4_string_property_callback string_cb,
			image4_validity_callback validity);


#endif