	KillAllTerminals()
	vim.cmd("wa")
	vim.wait(500)
			vim.cmd("tabnew")

local job_id = vim.fn.termopen(vim.o.shell)
vim.api.nvim_chan_send(job_id, "./run_server.sh\n")

			vim.cmd("vs")
vim.cmd("enew")

 job_id = vim.fn.termopen(vim.o.shell)
vim.api.nvim_chan_send(job_id, "./run_client.sh\n")

-- -- debug below
-- vim.api.nvim_chan_send(job_id, "cgdb ./generated/client.out\n")
-- vim.wait(500)
-- vim.api.nvim_chan_send(job_id, "b client.c:65\nr\nn\n")
