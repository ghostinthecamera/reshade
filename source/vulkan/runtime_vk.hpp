/**
 * Copyright (C) 2014 Patrick Mours. All rights reserved.
 * License: https://github.com/crosire/reshade#license
 */

#pragma once

#include "runtime.hpp"
#include "vk_handle.hpp"
#include "draw_call_tracker.hpp"

namespace reshade { enum class texture_reference; }
namespace reshadefx { struct sampler_info; }

namespace reshade::vulkan
{
	class runtime_vk : public runtime
	{
		friend struct vulkan_tex_data;
		friend struct vulkan_technique_data;
		friend struct vulkan_pass_data;
		friend struct vulkan_effect_data;

	public:
		runtime_vk(VkDevice device, VkPhysicalDevice physical_device, const VkLayerInstanceDispatchTable &instance_table, const VkLayerDispatchTable &device_table);

		bool on_init(VkSwapchainKHR swapchain, const VkSwapchainCreateInfoKHR &desc, HWND hwnd);
		void on_reset();
		void on_present(uint32_t swapchain_image_index, draw_call_tracker &tracker);

		bool capture_screenshot(uint8_t *buffer) const override;

#if RESHADE_VULKAN_CAPTURE_DEPTH_BUFFERS
		VkImage select_depth_texture_save(const VkImageCreateInfo &create_info);
#endif

		void transition_layout(
			VkCommandBuffer cmd_list, VkImage image, VkImageLayout old_layout, VkImageLayout new_layout,
			VkImageSubresourceRange subresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_ARRAY_LAYERS }) const;

		VkImage create_image(uint32_t width, uint32_t height, uint32_t levels, VkFormat format, VkImageUsageFlags usage, VkMemoryPropertyFlags mem = 0, VkImageCreateFlags = 0);
		VkImageView create_image_view(VkImage image, VkFormat format, uint32_t levels, VkImageAspectFlags aspect);
		VkBuffer create_buffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags mem = 0);
		VkCommandBuffer create_command_list(VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY) const;
		void execute_command_list(VkCommandBuffer cmd_list) const;
		void execute_command_list_async(VkCommandBuffer cmd_list) const;

		bool depth_buffer_before_clear = false;
		bool depth_buffer_more_copies = false;
		bool extended_depth_buffer_detection = false;
		unsigned int cleared_depth_buffer_index = 0;
		int depth_buffer_texture_format = VK_FORMAT_UNDEFINED; // No depth buffer texture format filter by default
		std::atomic<unsigned int> clear_DSV_iter = 1;

		VkDevice _device;
		VkPhysicalDevice _physical_device;
		VkPhysicalDeviceMemoryProperties _memory_props;
		VkSwapchainKHR _swapchain;

		VkLayerDispatchTable vk;

	private:
		bool init_texture(texture &texture);
		void upload_texture(texture &texture, const uint8_t *pixels);

		bool compile_effect(effect_data &effect);
		void unload_effect(size_t id);
		void unload_effects();

		bool init_technique(technique &info, VkShaderModule module, const VkSpecializationInfo &spec_info);

		void render_technique(technique &technique) override;

#if RESHADE_GUI
		bool init_imgui_resources();
		void render_imgui_draw_data(ImDrawData *draw_data) override;
		void draw_debug_menu();
#endif

#if RESHADE_VULKAN_CAPTURE_DEPTH_BUFFERS
		void detect_depth_source(draw_call_tracker &tracker);
		void update_depthstencil_image(VkImage depthstencil, VkImageLayout layout, VkFormat image_format);

		struct depth_texture_save_info
		{
			VkImage src_image;
			VkImageView src_depthstencil;
			VkImageCreateInfo src_image_info;
			VkImageViewCreateInfo src_image_view_info;
			VkImage dest_image;
			bool cleared = false;
		};
#endif

		void set_object_name(uint64_t handle, VkDebugReportObjectTypeEXT type, const char *name) const;

		void generate_mipmaps(const VkCommandBuffer cmd_list, texture &texture);

		VkFence _wait_fence = VK_NULL_HANDLE;
		VkQueue _current_queue = VK_NULL_HANDLE;
		uint32_t _swap_index = 0;

		std::vector<VkCommandPool> _cmd_pool;

		bool _is_multisampling_enabled = false;

		std::vector<VkImage> _swapchain_images;
		std::vector<VkImageView> _swapchain_views;
		std::vector<VkFramebuffer> _swapchain_frames;
		VkRenderPass _default_render_pass[2] = {};
		VkExtent2D _render_area = {};
		VkFormat _backbuffer_format = VK_FORMAT_UNDEFINED;

		std::vector<VkDeviceMemory> _allocations;

		VkImage _backbuffer_texture = VK_NULL_HANDLE;
		VkImageView _backbuffer_texture_view[2] = {};
		VkImage _default_depthstencil = VK_NULL_HANDLE;
		VkImageView _default_depthstencil_view = VK_NULL_HANDLE;
		VkImageView _depthstencil_shader_view = VK_NULL_HANDLE;
		VkImage _depthstencil_image = VK_NULL_HANDLE;
		VkImage _best_depth_stencil_overwrite = VK_NULL_HANDLE;
		VkImageLayout _depthstencil_layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		VkImageAspectFlags _depthstencil_aspect = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;

		std::unordered_map<uint64_t, VkImage> _depth_texture_saves;

		VkDescriptorPool _effect_descriptor_pool = VK_NULL_HANDLE;
		VkDescriptorSetLayout _effect_ubo_layout = VK_NULL_HANDLE;
		std::vector<struct vulkan_effect_data> _effect_data;
		std::unordered_map<size_t, VkSampler> _effect_sampler_states;

		draw_call_tracker *_current_tracker = nullptr;

#if RESHADE_GUI
		unsigned int _imgui_index_buffer_size = 0;
		VkBuffer _imgui_index_buffer = VK_NULL_HANDLE;
		unsigned int _imgui_vertex_buffer_size = 0;
		VkBuffer _imgui_vertex_buffer = VK_NULL_HANDLE;
		VkSampler _imgui_font_sampler = VK_NULL_HANDLE;
		VkPipeline _imgui_pipeline = VK_NULL_HANDLE;
		VkPipelineLayout _imgui_pipeline_layout = VK_NULL_HANDLE;
		VkDescriptorSetLayout _imgui_descriptor_set_layout = VK_NULL_HANDLE;
		VkDeviceSize _imgui_vertex_mem_offset = 0;
		VkDeviceMemory _imgui_vertex_mem = VK_NULL_HANDLE;
#endif
	};
}
