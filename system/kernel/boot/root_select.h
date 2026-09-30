/**
 * @file root_select.h
 * @brief Interface to KINIT root-controller discovery.
 */

#ifndef DAIMON_ROOT_SELECT_H
#define DAIMON_ROOT_SELECT_H

/**
 * @brief Sample the console selector and discover the requested boot root.
 *
 * Runs during MINIT processing, before kinit_boot() mounts the selected
 * filesystem.
 */
void root_select_minit(void);

/**
 * @brief Return the controller class chosen by root_select_minit().
 * @return Selected KINIT_ROOT_* class, or KINIT_ROOT_AUTO if unresolved.
 */
unsigned int root_select_class(void);

#endif
