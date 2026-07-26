	KillAllTerminals()
	vim.cmd("wa")
	vim.wait(500)
			vim.cmd("tabnew")

local job_id = vim.fn.termopen(vim.o.shell)

vim.api.nvim_chan_send(job_id, "./generated/main.out\n")

-- -- debug below
-- vim.api.nvim_chan_send(job_id, "cgdb ./generated/main.out\n")
-- vim.wait(500)
-- vim.api.nvim_chan_send(job_id, "")
-- vim.api.nvim_chan_send(job_id, "ib 78\nr\nn\n")
